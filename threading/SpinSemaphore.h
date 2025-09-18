/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of SpinSemaphore.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "Futex.h"
#include "make_load_order.h"
#include "utils/cpu_relax.h"
#include "utils/log2.h"
#include "utils/macros.h"
#include "utils/log2.h"
#include "utils/print_using.h"
#include "debug.h"
#include <mutex>

// Define this to print some stats on semaphore usage.
#undef SPINSEMAPHORE_STATS

#if defined(CWDEBUG) && !defined(DOXYGEN)
NAMESPACE_DEBUG_CHANNELS_START
extern channel_ct semaphore;
extern channel_ct semaphorestats;
NAMESPACE_DEBUG_CHANNELS_END
#endif

namespace utils::threading {

// class SpinSemaphore
//
// This semaphore has an internal state (existing of a single 64bit atomic)
// that keeps (atomically) track of the usual 'number of tokens' (ntokens >= 0),
// the 'number of potentially blocked threads' (nwaiters >= 0), and in addition
// to that one bit to keep track of whether or not it has a spinning thread.
//
// ntokens; the number of available tokens: this many threads can return from
//     wait() without adding more tokens, either because they already entered wait()
//     or because they enter wait later.
//
//     Obtaining a token (decrementing ntokens while ntokens > 0) is the last
//     thing a thread does before leaving wait() (except for the spinner, which
//     might call futex.wake(ntokens - 1) afterwards, see below).
//     While the first thing that post(n) does is incrementing ntokens with n.
//
// nwaiters; an upper limit of the number of threads that are or will end up being
//     blocked by a call to futex.wait() under the assumption that the number of
//     tokens do not change. Basically this number equals the number of threads that
//     entered wait() and did not obtain a token yet. It is incremented by one
//     immediately upon entering slow_wait() (unless there are tokens, then ntokens
//     is decremented instead and the thread leaves the function), and at exit of
//     wait() nwaiters and ntokens are atomically both decremented (unless there
//     are no tokens anymore, then neither is decremented and the thread stays
//     inside wait()).
//
// spinner; the thread that "owns" the spinner_mask bit. If that bit is not set
//     there is no spinner and when it is set there is a spinner. The thread that
//     owns the bit is the thread that set it. Hence there can be at most one thread
//     the spinner thread.
//
//     This thread must do the following: while ntokens == 0 it must keep spinning.
//     If ntokens > 0 it must attempt to, atomically, decrease the number of tokens
//     by one and reset the spinner bit, this can only fail when ntokens was changed
//     to zero again, in which case the spinner must return to spinning.
//     If it succeeds it must call futex.wake(ntokens - 1) (iff ntokens > 1),
//     where ntokens is the number of tokens just prior to the successful decrement.
//
//     In other words, tokens added to the atomic while the spinner bit is set causes
//     the spinner to take the responsiblity to wake up till that many additional
//     threads, if any.

class SpinSemaphore : public Futex<uint64_t>
{
 public:
  // The 64 bit of the atomic Futex<uint64_t>::m_word have the following meaning:
  //
  // |______________________________64_bit_word_____________________________|
  // |____________31 msb____________|_spin_|__32 least significant bits_____|
  //  [         nwaiters            ]   [S] [           ntokens             ]
  //                                 <----------nwaiters_shift------------->  = 33
  //  0000000000000000000000000000001    0  00000000000000000000000000000000  = 0x200000000 = one_waiter
  //  0000000000000000000000000000000    1  00000000000000000000000000000000  = 0x100000000 = spinner_mask
  //  0000000000000000000000000000000    0  11111111111111111111111111111111  =  0xffffffff = tokens_mask
  static constexpr int nwaiters_shift = 33;
  static constexpr uint64_t one_waiter = (uint64_t)1 << nwaiters_shift;
  static constexpr uint64_t spinner_mask = one_waiter >> 1;
  static constexpr uint64_t tokens_mask = spinner_mask - 1;
  static_assert(utils::log2(tokens_mask) < 32, "Must fit in a futex (32 bit).");

  // The memory order that we use to read-modify-write Futex<uint64_t>::m_word upon a successful compare_exchange_weak.
  static constexpr std::memory_order success_order = std::memory_order_acquire;

 protected:
  struct DelayLoop
  {
    static constexpr double delay_ms = 1.0;               // The required delay in ms.
    static constexpr double goal = 0.1;                   // Total time of delay loop while calibrating, in ms.
    static constexpr double time_per_loop = 1e-4;         // Shortest time between reads of atomic in loop, in ms (i.e. 100 ns).
    static constexpr int max_ols = goal / time_per_loop;  // Maximum value of outer loop size, during calibration.
    static constexpr int min_ols = max_ols / 4;           // Lowest value of outer loop size, during calibration.
    static constexpr unsigned int prefered_min_ils = 10;  // Reduce s_outer_loop_size till min_ols before going below prefered_min_ils.

    static unsigned int s_inner_loop_size;                // Inner loop size of delay loop.
    static unsigned int s_outer_loop_size;                // Outer loop size of delay loop.

    // This delay loop takes arguments for word, ols and ils for calibration purposes.
    // Returns when either ~1 ms passed or the least significant 32 bits in word changed to non-zero.
    // The last read value of word is returned.
    [[gnu::always_inline]] static uint64_t delay_loop(std::atomic<uint64_t>& word_ref, unsigned int ols, unsigned int ils)
    {
      uint64_t last_word;
      unsigned int i = ols;
      do
      {
        cpu_relax();
        if (((last_word = word_ref.load(std::memory_order_relaxed)) & tokens_mask) != 0)
          break;
        for (int j = ils; j != 0; --j)
          asm volatile ("");
      }
      while (--i != 0);
      return last_word;
    };

   public:
    // Calibrates delay loop, must be called once before use.
    static void calibrate(std::atomic<uint64_t>& word);

    static int outer_loop_size() { return s_outer_loop_size; }
    static int inner_loop_size() { return s_outer_loop_size; }
  };

 public:
  // Construct a SpinSemaphore with no waiters, no spinner and no tokens.
  SpinSemaphore() : Futex<uint64_t>(0)
#ifdef SPINSEMAPHORE_STATS
  , m_calls_to_post(0),
  m_tokens_added(0),
  m_calls_to_post_no_spinner(0),
  m_calls_to_futex_wake(0),
  m_calls_to_try_wait(0),
  m_failed_try_wait(0),
  m_calls_to_wait(0),
  m_calls_to_slow_wait(0),
  m_calls_to_futex_wait(0)
#endif
  {
    // Calibrate the delay loop once (using the m_word atomic of the first SpinSemaphore that is created).
    static std::once_flag flag;
    std::call_once(flag, [this]{ DelayLoop::calibrate(m_word); });
  }

  // Add n tokens to the semaphore.
  //
  // If there is no spinner but there are blocking threads (in wait()) then (at most) n threads
  // are woken up using a single system call. Each of those threads then will try to grab a token
  // before returning from wait(). If a thread fails to grab a token (because the tokens were
  // already grabbed by other threads that called wait() since) then they remain in wait() as
  // if not woken up.
  //
  // However, if there is a spinner thread then only that thread is woken up (without system call).
  // That thread will then wake up the additional threads, if any, before returning from wait().
  void post(uint32_t n = 1) noexcept
  {
    DoutEntering(dc::semaphore, "SpinSemaphore::post(" << n << ")");
    // Don't call post with n == 0.
    ASSERT(n >= 1);
    // Add n tokens.
    // A sem_wait needs to synchronize with the sem_post that provided the token, so that whatever
    // lead to the sem_post happens before the code after sem_wait.
    uint64_t const prev_word = m_word.fetch_add(n, std::memory_order_release);
    uint32_t const prev_ntokens = prev_word & tokens_mask;
    bool const have_spinner = prev_word & spinner_mask;

#ifdef SPINSEMAPHORE_STATS
    m_calls_to_post.fetch_add(1, std::memory_order_relaxed);
    m_tokens_added.fetch_add(n, std::memory_order_relaxed);
#endif

#if CW_DEBUG
    Dout(dc::semaphore, "Now " << utils::print_using(prev_word + n, print_word_on));
    // Check for possible overflow.
    ASSERT(prev_ntokens + n <= tokens_mask);
#endif

    // We avoid doing a syscall here, so if we have a spinner we're done.
    if (!have_spinner)
    {
#ifdef SPINSEMAPHORE_STATS
      m_calls_to_post_no_spinner.fetch_add(1, std::memory_order_relaxed);
#endif
      // No spinner was woken up, so we must do a syscall.
      //
      // Are there potential waiters that need to be woken up?
      uint32_t nwaiters = prev_word >> nwaiters_shift;
      if (nwaiters > prev_ntokens)
      {
#ifdef SPINSEMAPHORE_STATS
        m_calls_to_futex_wake.fetch_add(1, std::memory_order_relaxed);
#endif
        Dout(dc::semaphore, "Calling Futex<uint64_t>::wake(" << n << ") because nwaiters > prev_tokens (" << nwaiters << " > " << prev_ntokens << ").");
        DEBUG_ONLY(uint32_t woken_up =) Futex<uint64_t>::wake(n);
        Dout(dc::semaphore, "Woke up " << woken_up << " threads.");
        ASSERT(woken_up <= n);
      }
    }
  }

  // Try to remove a token from the semaphore.
  //
  // Returns a recently read value of m_word.
  // m_word was not changed by this function when (word & tokens_mask) == 0,
  // otherwise the number of tokens were decremented by one and returned
  // is the value of m_word immediately before this decrement.
  //
  // The returned value was loaded with std::memory_order_acquire.
  uint64_t fast_try_wait() noexcept
  {
    uint64_t word = m_word.load(make_load_order(success_order));
    do
    {
      uint64_t ntokens = word & tokens_mask;
      Dout(dc::semaphore, utils::print_using(word, print_word_on));
      // Are there any tokens to grab?
      if (ntokens == 0)
        return word;           // No debug output needed: if the above line prints tokens = 0 then this return is implied.
      // We seem to have a token, try to grab it.
    }
    while (!m_word.compare_exchange_weak(word, word - 1, success_order));
    // Token successfully grabbed.
    Dout(dc::semaphore, "Success, now " << ((word & tokens_mask) - 1) << " tokens left.");
    return word;
  }

  // Same as wait() but should only be called when (at least, very recently)
  // there are no tokens so that we are very likely to go to sleep.
  // word must be this very recently read value of m_word (so, (word & tokens_mask) should be 0).
  void slow_wait(uint64_t word) noexcept;

  // Block until a token can be grabbed.
  //
  // If no token is available then the thread will block until it manages to grab a new token (added with post(n) by another thread).
  void wait() noexcept
  {
    DoutEntering(dc::semaphore, "SpinSemaphore::wait()");
#ifdef SPINSEMAPHORE_STATS
    int ctw = m_calls_to_wait.fetch_add(1, std::memory_order_relaxed);
    if (ctw % 10000 == 0)
      Dout(dc::semaphorestats, *this);
#endif
    uint64_t word = fast_try_wait();
    if ((word & tokens_mask) == 0)
      slow_wait(word);
  }

  bool try_wait() noexcept
  {
    DoutEntering(dc::semaphore, "SpinSemaphore::try_wait()");
    bool success = fast_try_wait() & tokens_mask;
#ifdef SPINSEMAPHORE_STATS
    m_calls_to_try_wait.fetch_add(1, std::memory_order_relaxed);
    if (!success)
      m_failed_try_wait.fetch_add(1, std::memory_order_relaxed);
#endif
    return success;
  }

#ifdef CWDEBUG
  static void print_word_on(std::ostream& os, uint64_t word)
  {
    uint64_t nwaiters = word >> nwaiters_shift;
    bool spinner = word & spinner_mask;
    uint64_t ntokens = word & tokens_mask;
    os << "{waiters:" << nwaiters << ", spinner:" << spinner << ", tokens:" << ntokens << "}";
  }

  friend std::ostream& operator<<(std::ostream& os, SpinSemaphore const& spin_semaphore)
  {
    print_word_on(os, spin_semaphore.m_word.load(std::memory_order_relaxed));
    return os;
  };
#endif

#ifdef SPINSEMAPHORE_STATS
  std::atomic_int m_calls_to_post;
  std::atomic_int m_tokens_added;
  std::atomic_int m_calls_to_post_no_spinner;
  std::atomic_int m_calls_to_futex_wake;
  std::atomic_int m_calls_to_try_wait;
  std::atomic_int m_failed_try_wait;
  std::atomic_int m_calls_to_wait;
  std::atomic_int m_calls_to_slow_wait;
  std::atomic_int m_calls_to_futex_wait;

  void print_stats_on(std::ostream& os);
#endif
};

} // namespace utils::threading
