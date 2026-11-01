// SPDX-FileCopyrightText: 2026 Carlo Wood
// SPDX-License-Identifier: MIT

#pragma once

#include "utils/BitSet.h"
#include "utils/Array.h"
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <mutex>
#include <optional>

namespace utils::threading {

// class EventLoop
//
// Producer threads may call `notify` concurrently.
// Only one thread may call any of the wait, wait_for or wait_until member functions and access `result` afterwards.
//
// Notifications remain pending until returned by wait*, including notifications sent before wait*.
// Multiple pending notifications for one Event coalesce.
// Notifications received after an Event has been returned remain pending.
//
// terminate() causes the current or next wait* call to return `terminated`, with an empty event mask.
// Pending notifications are discarded.
// `terminate` blocks until the wait thread has acknowledged termination. It must not be called from the wait thread.
//
// All watch_* calls must occur before waiting begins.
// `notify` accepts only Events returned by `watch_event`.
// The caller must ensure that no producers are still using EventLoop when it is destroyed.
//
// `watch_fd` and `watch_event` throw std::runtime_error if too many events are registered.
//
// Usage:
//
//   using EventLoop = utils::threading::EventLoop;
//   using Event = EventLoop::Event;
//   using EventMask = EventLoop::EventMask;
//   using Index = EventLoop::Index;
//   using PollEvents = EventLoop::PollEvents;
//   using ReadyEvents = EventLoop::ReadyEvents;
//   using WaitStatus = EventLoop::WaitStatus;
//   using WaitResult = EventLoop::WaitResult;
//
//   EventLoop event_loop;
//   Event const e0 = event_loop.watch_fd(fd, PollEvents::read);
//   Index const i0 = EventLoop::index(e0);
//   Event const e1 = event_loop.watch_event();
//   Index const i1 = EventLoop::index(e1);
//
//   utils::Array<FooBar, max_events, Index> my_array;
//   my_array[i0] = stuff0;
//   my_array[i1] = stuff1;
//
//   for (;;)
//   {
//     event_loop.wait();
//     WaitResult const& wait_result = event_loop.result();
//     EventMask events = wait_result.events();
//     if (events.any())
//     {
//       for (auto event : events)
//       {
//         Index index = EventLoop::index(event);
//         FooBar& foobar = my_array[index];
//         if (event_loop.is_fd_event(event))
//         {
//           int fd = wait_result.fd(index);
//           ReadyEvents ready_events = wait_result.ready_events(index);
//           // ... handle I/O.
//         }
//         else
//         {
//           // ... handle event.
//         }
//       }
//     }
//     else if (wait_result.status() == WaitStatus::terminated)
//       break;
//     else
//     {
//       // ...otherwise this would be a timeout.
//       continue;
//     }
//   }
//
class EventLoop
{
 public:
  using events_mask_type = uint_fast32_t;               // At least 32 bits.
  using Event = BitSet<events_mask_type>;               // A single bit representing one distinct event as registered with watch_*.
  using EventMask = BitSet<events_mask_type>;           // One or more bits representing distinct events.
  using EventIndex = Event::Index;                      // The bit index of an Event.
  using Index = ArrayIndex<EventLoop>;                  // The array index of an Event.

  static constexpr events_mask_type no_events{0};
  static constexpr int max_number_of_events =           // The maximum number of distinct events that may be watched.
    std::numeric_limits<events_mask_type>::digits;

  enum class PollEvents : unsigned int
  {
    read    = 1u << 0,                                  // The fd is readable.
    write   = 1u << 1                                   // The fd is writable.
  };

  enum class ReadyEvents : unsigned int
  {
    read    = 1u << 0,                                  // The fd is readable.
    write   = 1u << 1,                                  // The fd is writable.
    hangup  = 1u << 2,                                  // The other end of a communication channel has closed or disconnected.
    error   = 1u << 3                                   // The underlying object has an error condition.
  };

  friend constexpr PollEvents operator|(PollEvents a, PollEvents b)
  {
    return static_cast<PollEvents>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
  }

  friend constexpr ReadyEvents operator|(ReadyEvents a, ReadyEvents b)
  {
    return static_cast<ReadyEvents>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
  }

  friend constexpr bool has_event(ReadyEvents events, ReadyEvents event)
  {
    return (static_cast<unsigned int>(events) & static_cast<unsigned int>(event)) != 0;
  }

  enum class WaitStatus
  {
    pending_events,             // WaitResult::events will return a non-empty mask (EventMask::any() is true).
    timed_out,                  // WaitResult::events will return an empty mask (EventMask::none() is true).
    terminated                  // idem
  };

  class WaitResult
  {
   public:
    WaitStatus status() const { return status_; }
    EventMask events() const { return events_; }

    // Valid only if for a fd event present in events().
    ReadyEvents ready_events(Index event_index) const { return ready_[event_index]; }
    int fd(Index event_index) const { return fds_[event_index]; }

   private:
    friend class EventLoop;

    WaitStatus status_ = WaitStatus::timed_out;
    EventMask events_{EventLoop::no_events};
    Array<ReadyEvents, max_number_of_events, Index> ready_{};
    Array<int, max_number_of_events, Index> fds_{};

    void clear()
    {
      status_ = WaitStatus::timed_out;
      events_ = EventMask{EventLoop::no_events};
      ready_ = {};
    }
  };

 private:
  EventMask fd_events_{no_events};              // Bit mask with events that are associated with fds.
  EventIndex next_event_{bitset::index_begin};  // The next event to return from a call to watch*.
                                                // Numerically equal to the number of currently registered events.
  WaitResult result_;                           // This object holds the result of each wait* call.

  struct Registration
  {
    int fd = -1;
    PollEvents interests{};
  };

  // Event registration.
  Array<Registration, max_number_of_events, Index> registrations_;

  // Platform-specific event queue.
  int poller_fd_ = -1;                          // The epoll descriptor on Linux or the kqueue descriptor on BSD/macOS
#ifdef __linux__
  int wake_fd_ = -1;                            // The eventfd(2) descriptor. The BSD backend uses EVFILT_USER.
#endif
  std::atomic<events_mask_type> pending_{0};

  // Termination.
  std::atomic<bool> terminating_{false};
  std::mutex termination_mutex_;
  std::condition_variable termination_cv_;
  bool termination_acknowledged_ = false;

  void wait_impl(std::optional<std::chrono::nanoseconds> timeout);
  void signal_wakeup();

 public:
  // Constructor.
  EventLoop();
  ~EventLoop();

  // An event loop can not be copied.
  EventLoop(EventLoop const&) = delete;
  EventLoop& operator=(EventLoop const&) = delete;

  // Register a new Event for when `fd` becomes ready for one or more of the registered `interests`, or one of hangup or error.
  [[nodiscard]] Event watch_fd(int fd, PollEvents interests);

  // Register a new Event.
  [[nodiscard]] Event watch_event();

  // Convert a single-bit Event to the corresponding array Index.
  static Index index(Event event)
  {
    return Index{static_cast<int>(event.lssbi()())};
  }

  //---------------------------------------------------------------------------
  // Called by the 'wait' thread.

  // Block until one of the watched events occurs.
  void wait()
  {
    return wait_impl(std::nullopt);
  }

  // Block until one of the watched events occurs, the given duration has been elapsed.
  template<typename Rep, typename Period>
  void wait_for(std::chrono::duration<Rep, Period> const& rel_time);

  // Block until one of the watched events occurs, the given time point has been reached.
  template<typename Clock, typename Duration>
  void wait_until(std::chrono::time_point<Clock, Duration> const& abs_time);

  // Get a reference to the result object, mutated by the last call to wait*.
  // Note that if a wait* call throws, then the result contents are unspecified.
  WaitResult const& result() const { return result_; }

  // Return true iff `event` is an Event that was previously returned by `watch_fd`.
  bool is_fd_event(Event event) const { return (fd_events_ & event) == event; }

  // Return the number of registered events.
  std::size_t number_of_events() const { return static_cast<std::size_t>(next_event_()); }

  //---------------------------------------------------------------------------
  // Called by 'notify' threads.

  // Publish a notification for `event` with release semantics.
  // The wait* call that consumes this notification acquires it and returns with the corresponding event bit set.
  void notify(Event event);

  // Cause the current, or the next call to wait* to return `WaitStatus::terminated`.
  void terminate();
};

template<typename Rep, typename Period>
void EventLoop::wait_for(std::chrono::duration<Rep, Period> const& rel_time)
{
  if (rel_time <= rel_time.zero())
  {
    wait_impl(std::chrono::nanoseconds::zero());
    return;
  }

  using Clock = std::chrono::steady_clock;
  auto const now = Clock::now();
  auto const max_wait = Clock::time_point::max() - now;

  if (std::chrono::duration<long double>(rel_time) >= std::chrono::duration<long double>(max_wait))
    wait_until(Clock::time_point::max());
  else
    wait_until(now + std::chrono::ceil<Clock::duration>(rel_time));
}

template<typename Clock, typename Duration>
void EventLoop::wait_until(std::chrono::time_point<Clock, Duration> const& abs_time)
{
  for (;;)
  {
    auto const now = Clock::now();
    std::chrono::nanoseconds duration{};

    if (abs_time > now)
    {
      auto const remaining = abs_time - now;

      duration = remaining > std::chrono::hours{1} ?
          std::chrono::hours{1} :
          std::chrono::ceil<std::chrono::nanoseconds>(remaining);
    }

    wait_impl(duration);

    if (result_.status() != WaitStatus::timed_out || Clock::now() >= abs_time)
      return;
  }
}

} // namespace utils::threading
