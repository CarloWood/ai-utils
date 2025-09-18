/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of SpinSemaphore.
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

#include "sys.h"
#include "SpinSemaphore.h"
#include "utils/DelayLoopCalibration.h"
#include "utils/print_using.h"
#include <cmath>
#ifdef SPINSEMAPHORE_STATS
#include <iomanip>
#endif

namespace utils::threading {

void SpinSemaphore::slow_wait(uint64_t word) noexcept
{
#ifdef SPINSEMAPHORE_STATS
  m_calls_to_slow_wait.fetch_add(1, std::memory_order_relaxed);
#endif

  // Calling slow_wait implies we have (had) no tokens, hence this should be true.
  // Don't call slow_wait unless this is true.
  ASSERT((word & tokens_mask) == 0);

  bool already_had_spinner;
  uint64_t ntokens;
  uint64_t new_word;
  do
  {
    already_had_spinner = (word & spinner_mask);
    ntokens = word & tokens_mask;
    new_word = !ntokens ? (word + one_waiter)         // We are (likely) going to block: add one to the number of waiters.
                          | spinner_mask              // Also, if there isn't already a spinner, grab the spinner bit.
                        : word - 1;                   // Someone added a new token before we even could go to sleep. Try to grab it!
  }
  while (!m_word.compare_exchange_weak(word, new_word, success_order));
  bool we_are_spinner = false;

  // Wait for a token to be available. Retry until we can grab one.
  for (;;)
  {
    if (AI_UNLIKELY(ntokens > 0))
    {
      // There were new tokens and we managed to grab one.
      Dout(dc::semaphore, "Successfully obtained a token. Now " << (ntokens - 1) << " tokens and " << ((word >> nwaiters_shift) - 1) << " waiters left.");
      return;
    }

    // At this point, !already_had_spinner means that we grabbed the spinner bit.
    we_are_spinner |= !already_had_spinner;

    // If there is no token available, block until a new token was added.

    if (!we_are_spinner)
    {
      Dout(dc::semaphore, "Already had a spinner; calling Futex::wait.");
      // Already had a spinner, go to sleep.
      //
      // As of kernel 2.6.22 FUTEX_WAIT only returns with -1 when the syscall was
      // interrupted by a signal. In that case errno should be set to EINTR.
      // [Linux kernels before 2.6.22 could also return EINTR upon a spurious wakeup,
      // in which case it is also OK to just reenter wait() again.]
      [[maybe_unused]] int res;
futex_sleep:
#ifdef SPINSEMAPHORE_STATS
      m_calls_to_futex_wait.fetch_add(1, std::memory_order_relaxed);
#endif
      while ((res = Futex<uint64_t>::wait(0)) == -1 && errno != EAGAIN)
        ;
      // EAGAIN happens when the number of tokens was changed in the meantime.
      // We (supuriously?) woke up or failed to go to sleep because the number of tokens changed.
      // It is therefore not sure that there is a token for us. Refresh word and try again.
      word = m_word.load(make_load_order(success_order));
      Dout(dc::semaphore(res == 0), "Woke up! tokens = " << (word & tokens_mask) << "; waiters = " << (word >> nwaiters_shift));
      // We woke up, try again to get a token.
      do
      {
        already_had_spinner = (word & spinner_mask);
        ntokens = word & tokens_mask;
        Dout(dc::semaphore, "Seeing " << ntokens << " tokens and " << (word >> nwaiters_shift) << " waiters.");
        new_word = !ntokens ? word | spinner_mask
                            : word - one_waiter - 1;    // (Try to) atomically grab a token and stop being a waiter.
#ifdef DEBUGGENMC
        if (res == 0)
          new_word -= futex_woke_bit;                   // We're done grabbing a token (if any) after waking up from Futex::wait.
        if (0)
#endif
        // There is no need to do the CAS below when it would be a nop.
        if (!ntokens && already_had_spinner)
          break;
      }
      while (!m_word.compare_exchange_weak(word, new_word, success_order));
      // If ntokens > 0 here then we successfully grabbed one, otherwise
      // if already_had_spinner is false then we successfully became the spinner ourselves.
      // Go to the top of the loop to handle both situations...
    }
    else
    {
      Dout(dc::semaphore|continued_cf, "Became spinner; ");
      do
      {
        // We are the spinner. Spin instead of going to sleep.
//        Dout(dc::continued, "entering delay loop... ");
//        auto start = std::chrono::high_resolution_clock::now();
        // Spin for 20 milliseconds...
        word = DelayLoop::delay_loop(m_word, 20 * DelayLoop::s_outer_loop_size, DelayLoop::s_inner_loop_size);
//        auto end = std::chrono::high_resolution_clock::now();
//        std::chrono::duration<double> diff = end - start;

        for (;;)
        {
          ntokens = word & tokens_mask;
          if (AI_UNLIKELY(ntokens > 0))         // New tokens were added...
            break;
          // ...or did we time out?
          new_word = word & ~spinner_mask;      // Unbecome the spinner.
          if (!m_word.compare_exchange_weak(word, new_word, std::memory_order_relaxed))
            continue;
//          Dout(dc::finish, "timed out (" << (1000 * diff.count()) << " ms). No longer the spinner.");
          Dout(dc::finish, "timed out. No longer the spinner.");
          we_are_spinner = false;
          goto futex_sleep;
        }

        // Before we execute the following CAS new idle threads can decrement ntokens,
        // and because they subsequently continue running instead of going to sleep we'd
        // have to wake up less threads.
        //
        // It is also possible that ntokens is incremented because post() is called
        // while we are here; this then is still our reponsibility: as long as the
        // spinner bit is set we are responsible for waking up m_word & tokens_mask threads.
        //
        // If ntokens is reduced to 0 before we execute the following line then we must
        // continue to be the spinner.

        // Try to grab a token for ourselves, reset being the spinner and decrement the number
        // of waiters atomically, for as long as there are new tokens available.
        do
        {
          new_word = word - 1 - spinner_mask - one_waiter;
#ifdef DEBUGGENMC
          if ((word >> nwaiters_shift) > 0 && ntokens > 1)
            new_word += futex_wake_bit;                                                 // Mark that we are going to call Futex::wake.
#endif
        }
        while (!m_word.compare_exchange_weak(word, new_word, success_order) &&
               (ntokens = (word & tokens_mask)) > 0);
#ifdef CWDEBUG
        if (ntokens > 0)
          Dout(dc::finish, "Grabbed a token; no longer the spinner. Now " << utils::print_using(new_word, print_word_on));
#endif
      }
      while (ntokens == 0);
      // We must wake up ntokens - 1 threads.
      // Are there potential waiters that need to be woken up?
      uint32_t nwaiters = word >> nwaiters_shift;
      if (nwaiters > 0 && ntokens > 1)
      {
        Dout(dc::semaphore, "Calling Futex<uint64_t>::wake(" << (ntokens - 1) << ") because there were waiters (" << nwaiters << ").");
#ifdef SPINSEMAPHORE_STATS
        m_calls_to_futex_wake.fetch_add(1, std::memory_order_relaxed);
#endif
        DEBUG_ONLY(uint32_t woken_up =) Futex<uint64_t>::wake(ntokens - 1);
#ifdef DEBUGGENMC
        atomic_fetch_sub_explicit(&m_word, futex_wake_bit, memory_order_release);       // Done calling Futex::wake.
#endif
        Dout(dc::semaphore, "Woke up " << woken_up << " threads.");
        ASSERT(woken_up <= ntokens);
      }
      return;
    }
  }
}

//static
unsigned int SpinSemaphore::DelayLoop::s_outer_loop_size;
unsigned int SpinSemaphore::DelayLoop::s_inner_loop_size;

void SpinSemaphore::DelayLoop::calibrate(std::atomic<uint64_t>& word)
{
  DoutEntering(dc::notice, "SpinSemaphore::DelayLoop::calibrate()");

  // Define delay loop lambdas. This requires C++17.
  utils::DelayLoopCalibration fixed_ols_delay_loop([&word](unsigned int ils){ return delay_loop(word, s_outer_loop_size, ils); });
  utils::DelayLoopCalibration fixed_ils_delay_loop([&word](unsigned int ols){ return delay_loop(word, ols, s_inner_loop_size); });

  // Determine s_outer_loop_size, with s_inner_loop_size fixed at prefered_min_ils, for a delay time of goal ms.
  s_inner_loop_size = prefered_min_ils;
  s_outer_loop_size = fixed_ils_delay_loop.peak_detect(goal);
  Dout(dc::delayloop, "s_outer_loop_size (with s_inner_loop_size = " << s_inner_loop_size << ") = " << s_outer_loop_size);

  // Find an appropriate value for s_inner_loop_size.
  if (s_outer_loop_size < min_ols)
  {
    Dout(dc::warning, "SpinSemaphore::DelayLoop::calibrate: s_outer_loop_size is less than min_ols (" << min_ols << "), will need to use an s_inner_loop_size less than " << prefered_min_ils << "!");
    // Find the largest s_inner_loop_size (albeit smaller than prefered_min_ils),
    // with s_outer_loop_size fixed at min_ols, such that the delay loop at least goal ms.
    s_outer_loop_size = min_ols;
    for (s_inner_loop_size = 0; s_inner_loop_size < prefered_min_ils; ++s_inner_loop_size)
      if (fixed_ols_delay_loop.avg_of(s_inner_loop_size) > goal)
        break;
    Dout(dc::delayloop, "s_inner_loop_size (with s_outer_loop_size = " << s_outer_loop_size << ") = " << s_inner_loop_size);
  }
  else if (s_outer_loop_size > max_ols)
  {
    // Fix s_outer_loop_size at 90% of its maximum value and find the corresponding best s_inner_loop_size for that.
    s_outer_loop_size = 0.9 * max_ols;
    s_inner_loop_size = fixed_ols_delay_loop.search_lowest_of(20, goal);
    Dout(dc::delayloop, "s_inner_loop_size (with s_outer_loop_size = " << s_outer_loop_size << ") = " << s_inner_loop_size);
  }
  else
  {
    Dout(dc::notice, "SpinSemaphore::DelayLoop::calibrate: s_outer_loop_size is less than max_ols (" << max_ols << ") while using s_inner_loop_size of only " << s_inner_loop_size << " (slow hardware?)");
    // Assuming that s_inner_loop_size behaves linear (it actually might, under 10), namely
    //
    //   goal = alpha * s_outer_loop_size * (beta + gamma * s_inner_loop_size),
    //
    // we know that multiplying s_inner_loop_size with a factor s_outer_loop_size / max_ols and setting s_outer_loop_size
    // to max_ols we'd get a delay of:
    //
    //   alpha * max_ols * (beta + gamma * s_inner_loop_size * (s_outer_loop_size / max_ols)) = goal + alpha * beta * (max_ols - s_outer_loop_size)
    //
    // So, the delay would become larger than goal and the s_outer_loop_size corresponding
    // to goal will thus be smaller than max_ols.
    unsigned int prev_ils = s_inner_loop_size;
    s_inner_loop_size = (1.0 * s_outer_loop_size / max_ols) * s_inner_loop_size;
    while (s_inner_loop_size < prev_ils)
    {
      // Find a corresponding s_outer_loop_size.
      s_outer_loop_size = fixed_ils_delay_loop.peak_detect(goal /*, "Delay with ils = " + std::to_string(s_inner_loop_size)*/);
      Dout(dc::delayloop, "s_outer_loop_size (with s_inner_loop_size = " << s_inner_loop_size << ") = " << s_outer_loop_size);
      if (s_outer_loop_size > min_ols)
      {
        ++s_inner_loop_size;
        break;
      }
      prev_ils = s_inner_loop_size;
      s_inner_loop_size = (1.0 * s_outer_loop_size / max_ols) * s_inner_loop_size;
    }
  }

  // Finally, find the best s_outer_loop_size with this s_inner_loop_size.
  s_outer_loop_size = fixed_ils_delay_loop.peak_detect(goal
      COMMA_CWDEBUG_ONLY("Delay with goal " + std::to_string(goal) + " ms and ils = " + std::to_string(s_inner_loop_size)));
  Dout(dc::delayloop, "s_outer_loop_size (with s_inner_loop_size = " << s_inner_loop_size << ") = " << s_outer_loop_size);

  // Set s_outer_loop_size to a value that should lead to a delay_ms delay.
  s_outer_loop_size *= std::round(delay_ms / goal);

  Dout(dc::notice, "Delay loop calibration: inner_loop_size = " << s_inner_loop_size <<
      "; outer_loop_size = " << s_outer_loop_size << "; test run: " << fixed_ils_delay_loop.avg_of(s_outer_loop_size) << " ms.");
}

#ifdef SPINSEMAPHORE_STATS
void SpinSemaphore::print_stats_on(std::ostream& os)
{
  double avg_tokens_added = 1.0 * m_tokens_added / m_calls_to_post;
  os << "SpinSemaphore stats:\nCalls to post: " << m_calls_to_post << " (" << m_tokens_added << " tokens in total: " << std::fixed << std::setprecision(2) << avg_tokens_added << " tokens on average per call).\n";
  os << "Calls to post, no spinner: " << m_calls_to_post_no_spinner << '\n';
  os << "Calls to Futex::wake: " << m_calls_to_futex_wake << '\n';
  os << "Calls to try_wait: " << m_calls_to_try_wait << " (failed: " << m_failed_try_wait << " = " << (100.0 * m_failed_try_wait / m_calls_to_try_wait) << "%).\n";
  os << "Calls to wait: " << m_calls_to_wait << '\n';
  os << "Calls to slow_wait: " << m_calls_to_slow_wait << " (" << std::fixed << std::setprecision(2) << (100.0 * m_calls_to_slow_wait / m_calls_to_wait) << "%).\n";
  os << "Calls to Futex::wait: " << m_calls_to_futex_wait << '\n';
}
#endif

} // namespace utils::threading

#if defined(CWDEBUG) && !defined(DOXYGEN)
NAMESPACE_DEBUG_CHANNELS_START
channel_ct semaphore("SEMAPHORE");
channel_ct semaphorestats("SEMAPHORESTATS");
NAMESPACE_DEBUG_CHANNELS_END
#endif
