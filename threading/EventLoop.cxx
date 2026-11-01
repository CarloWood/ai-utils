// SPDX-FileCopyrightText: 2026 Carlo Wood
// SPDX-License-Identifier: MIT

#include "sys.h"
#include "EventLoop.h"

#include <array>
#include <cerrno>
#include <cstdint>
#include <stdexcept>
#include <system_error>
#include <unistd.h>

#if defined(__linux__)
#include <sys/epoll.h>
#include <sys/eventfd.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__)
#include <fcntl.h>
#include <sys/event.h>
#include <sys/time.h>
#else
#error "EventLoop requires Linux epoll or a BSD-derived kqueue implementation."
#endif

namespace utils::threading {

namespace {

[[noreturn]] void throw_system_error(char const* operation)
{
  throw std::system_error(errno, std::generic_category(), operation);
}

#if defined(__linux__)
constexpr uint64_t wakeup_id = EventLoop::max_number_of_events;
#else
constexpr uintptr_t wakeup_id = 1;
#endif

} // namespace

EventLoop::EventLoop()
{
#if defined(__linux__)
  poller_fd_ = ::epoll_create1(EPOLL_CLOEXEC);

  if (poller_fd_ == -1)
    throw_system_error("epoll_create1");

  wake_fd_ = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);

  if (wake_fd_ == -1)
  {
    int error = errno;
    ::close(poller_fd_);
    throw std::system_error(error, std::generic_category(), "eventfd");
  }

  epoll_event e{};
  e.events = EPOLLIN;
  e.data.u64 = wakeup_id;

  if (::epoll_ctl(poller_fd_, EPOLL_CTL_ADD, wake_fd_, &e) == -1)
  {
    int error = errno;
    ::close(wake_fd_);
    ::close(poller_fd_);
    throw std::system_error(error, std::generic_category(), "epoll_ctl(eventfd)");
  }
#else
  poller_fd_ = ::kqueue();

  if (poller_fd_ == -1)
    throw_system_error("kqueue");

  if (::fcntl(poller_fd_, F_SETFD, FD_CLOEXEC) == -1)
  {
    int error = errno;
    ::close(poller_fd_);
    throw std::system_error(error, std::generic_category(), "fcntl(FD_CLOEXEC)");
  }

  struct kevent change;
  EV_SET(&change, wakeup_id, EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, nullptr);

  if (::kevent(poller_fd_, &change, 1, nullptr, 0, nullptr) == -1)
  {
    int error = errno;
    ::close(poller_fd_);
    throw std::system_error(error, std::generic_category(), "kevent(EVFILT_USER)");
  }
#endif
}

EventLoop::~EventLoop()
{
#if defined(__linux__)
  ::close(wake_fd_);
#endif
  ::close(poller_fd_);
}

EventLoop::Event EventLoop::watch_event()
{
  if (number_of_events() == max_number_of_events)
    throw std::runtime_error("Too many EventLoop events");

  Event event{next_event_++};
  return event;
}

EventLoop::Event EventLoop::watch_fd(int fd, PollEvents interests)
{
  if (fd < 0)
    throw std::invalid_argument("watch_fd: negative descriptor");

  unsigned int const wanted = static_cast<unsigned int>(interests);
  unsigned int const valid = static_cast<unsigned int>(PollEvents::read | PollEvents::write);

  if ((wanted & valid) == 0 || (wanted & ~valid) != 0)
    throw std::invalid_argument("watch_fd: invalid interests");

  if (number_of_events() == max_number_of_events)
    throw std::runtime_error("Too many EventLoop events");

  // Reusing the same descriptor would overwrite registrations on kqueue.
  for (Index i{0}; i < Index{static_cast<int>(number_of_events())}; ++i)
  {
    if (registrations_[i].fd == fd)
      throw std::invalid_argument("watch_fd: descriptor already registered");
  }

  Event event{next_event_};
  Index i = index(event);
  Registration& registration = registrations_[i];
  registration.fd = fd;
  registration.interests = interests;

#if defined(__linux__)
  epoll_event change{};
  change.events = EPOLLRDHUP;

  if ((wanted & static_cast<unsigned int>(PollEvents::read)) != 0)
    change.events |= EPOLLIN;
  if ((wanted & static_cast<unsigned int>(PollEvents::write)) != 0)
    change.events |= EPOLLOUT;

  change.data.u64 = static_cast<uint64_t>(i.get_value());

  if (::epoll_ctl(poller_fd_, EPOLL_CTL_ADD, fd, &change) == -1)
  {
    int error = errno;
    registration = Registration{};
    throw std::system_error(error, std::generic_category(), "epoll_ctl(EPOLL_CTL_ADD)");
  }
#else
  // Register the two kqueue filters separately but with the same stable address.
  struct kevent change;
  bool added_read = false;

  if ((wanted & std::to_underlying(PollEvents::read)) != 0)
  {
    EV_SET(&change, fd, EVFILT_READ, EV_ADD, 0, 0, &registration);

    if (::kevent(poller_fd_, &change, 1, nullptr, 0, nullptr) == -1)
    {
      registration = Registration{};
      throw_system_error("kevent(EVFILT_READ)");
    }
    added_read = true;
  }

  if ((wanted & std::to_underlying(PollEvents::write)) != 0)
  {
    EV_SET(&change, fd, EVFILT_WRITE, EV_ADD, 0, 0, &registration);

    if (::kevent(poller_fd_, &change, 1, nullptr, 0, nullptr) == -1)
    {
      int error = errno;

      if (added_read)
      {
        EV_SET(&change, fd, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        ::kevent(poller_fd_, &change, 1, nullptr, 0, nullptr);
      }
      registration = Registration{};
      throw std::system_error(error, std::generic_category(), "kevent(EVFILT_WRITE)");
    }
  }
#endif

  fd_events_ |= event;
  ++next_event_;
  return event;
}

void EventLoop::signal_wakeup()
{
#if defined(__linux__)
  uint64_t const one = 1;

  for (;;)
  {
    ssize_t const written = ::write(wake_fd_, &one, sizeof(one));

    if (written == sizeof(one))
      return;
    if (written == -1 && errno == EINTR)
      continue;
    if (written == -1 && errno == EAGAIN)
      return;  // The counter is full, hence the descriptor is already readable.

    if (written == -1)
      throw_system_error("write(eventfd)");
    throw std::runtime_error("short write(eventfd)");
  }
#else
  struct kevent change;
  EV_SET(&change, wakeup_id, EVFILT_USER, 0, NOTE_TRIGGER, 0, nullptr);

  for (;;)
  {
    if (::kevent(poller_fd_, &change, 1, nullptr, 0, nullptr) == 0)
      return;
    if (errno != EINTR)
      throw_system_error("kevent(NOTE_TRIGGER)");
  }
#endif
}

void EventLoop::notify(Event event)
{
  if (terminating_.load(std::memory_order_acquire))
    return;

  pending_.fetch_or(event(), std::memory_order_release);
  signal_wakeup();
}

EventLoop::WaitResult EventLoop::finish_wait(WaitResult result)
{
  if (!terminating_.load(std::memory_order_acquire))
    return result;

  pending_.exchange(0, std::memory_order_acquire);

  result = WaitResult{};
  result.status_ = WaitStatus::terminated;

  {
    std::lock_guard lock(termination_mutex_);
    termination_acknowledged_ = true;
  }
  termination_cv_.notify_all();
  return result;
}

EventLoop::WaitResult EventLoop::wait_impl(std::optional<std::chrono::nanoseconds> timeout)
{
  using Clock = std::chrono::steady_clock;

  std::optional<Clock::time_point> deadline;

  if (timeout)
  {
    auto const now = Clock::now();
    auto const max_wait = Clock::time_point::max() - now;
    deadline = *timeout >= max_wait ? Clock::time_point::max() : now + *timeout;
  }

  for (;;)
  {
    if (terminating_.load(std::memory_order_acquire))
      return finish_wait(WaitResult{});

    timespec ts{};
    timespec const* pts = nullptr;

    if (deadline)
    {
      auto const now = Clock::now();
      auto const remaining = now < *deadline ? std::chrono::duration_cast<std::chrono::nanoseconds>(*deadline - now) : std::chrono::nanoseconds::zero();
      ts.tv_sec = static_cast<time_t>(std::chrono::duration_cast<std::chrono::seconds>(remaining).count());
      ts.tv_nsec = static_cast<long>((remaining % std::chrono::seconds{1}).count());
      pts = &ts;
    }

    WaitResult result;
#if defined(__linux__)
    std::array<epoll_event, max_number_of_events + 1> ready{};
    int const n = ::epoll_pwait2(poller_fd_, ready.data(), ready.size(), pts, nullptr);
#else
    // A descriptor may produce distinct read and write results.
    std::array<struct kevent, 2 * max_number_of_events + 1> ready{};
    int const n = ::kevent(poller_fd_, nullptr, 0, ready.data(), ready.size(), pts);
#endif

    if (n == -1)
    {
      if (errno == EINTR)
        continue;
      throw_system_error("event queue wait");
    }

#if defined(__linux__)
    bool woken = false;
#endif

    for (int j = 0; j < n; ++j)
    {
#if defined(__linux__)
      epoll_event const& ev = ready[j];
      if (ev.data.u64 == wakeup_id)
      {
        woken = true;
        continue;
      }

      Index i{static_cast<int>(ev.data.u64)};
      ReadyEvents flags{};

      if (ev.events & EPOLLIN)
        flags = flags | ReadyEvents::read;
      if (ev.events & EPOLLOUT)
        flags = flags | ReadyEvents::write;
      if (ev.events & (EPOLLHUP | EPOLLRDHUP))
        flags = flags | ReadyEvents::hangup;
      if (ev.events & EPOLLERR)
        flags = flags | ReadyEvents::error;
#else
      struct kevent const& ev = ready[j];
      if (ev.filter == EVFILT_USER && ev.ident == wakeup_id)
        continue;

      auto const* registration = static_cast<Registration const*>(ev.udata);
      Index i{static_cast<int>(registration - registrations_.data())};
      ReadyEvents flags{};

      if (ev.filter == EVFILT_READ)
        flags = flags | ReadyEvents::read;
      if (ev.filter == EVFILT_WRITE)
        flags = flags | ReadyEvents::write;
      if (ev.flags & EV_EOF)
        flags = flags | ReadyEvents::hangup;
      if (ev.flags & EV_ERROR)
        flags = flags | ReadyEvents::error;
#endif

      result.events_ |= Event{EventIndex{bitset::IndexPOD{static_cast<int8_t>(i.get_value())}}};
      result.ready_[i] = result.ready_[i] | flags;
      result.fds_[i] = registrations_[i].fd;
    }

#if defined(__linux__)
    // Drain the persistent wakeup before clearing the pending bits.
    if (woken)
    {
      uint64_t value;
      for (;;)
      {
        ssize_t const nread = ::read(wake_fd_, &value, sizeof(value));
        if (nread == sizeof(value) || (nread == -1 && errno == EAGAIN))
          break;
        if (nread == -1 && errno == EINTR)
          continue;
        if (nread == -1)
          throw_system_error("read(eventfd)");
        throw std::runtime_error("short read(eventfd)");
      }
    }
#endif

    result.events_ |= pending_.exchange(0, std::memory_order_acquire);

    if (result.events_.any())
    {
      result.status_ = WaitStatus::pending_events;
      return finish_wait(result);
    }

    if (deadline && Clock::now() >= *deadline)
      return finish_wait(result);
  }
}

void EventLoop::terminate()
{
  if (!terminating_.exchange(true, std::memory_order_acq_rel))
    signal_wakeup();

  std::unique_lock lock(termination_mutex_);
  termination_cv_.wait(lock, [this] { return termination_acknowledged_; });
}

} // namespace utils::threading
