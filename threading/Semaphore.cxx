// SPDX-FileCopyrightText: 2021, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of Semaphore.
 */

#include "sys.h"
#include "Semaphore.h"

namespace utils::threading
{

void Semaphore::slow_wait() noexcept
{
  // We are (likely) going to block. Add one to the number of waiters.
  uint64_t word = m_word.fetch_add(one_waiter, std::memory_order_relaxed) + one_waiter;

  // Wait for a token to be available. Retry until we can grab one.
  for (;;)
  {
    // If there is no token available, block until a new token was added.
    uint32_t ntokens = word & tokens_mask;
    Dout(dc::semaphore, "Seeing " << ntokens << " tokens and " << (word >> nwaiters_shift) << " waiters.");
    if (ntokens == 0)
    {
      // As of kernel 2.6.22 FUTEX_WAIT only returns with -1 when the syscall was
      // interrupted by a signal. In that case errno should be set to EINTR.
      // Linux kernels before 2.6.22 could also return EINTR upon a supurious wakeup,
      // in which case it is also OK to just reenter wait() again.
      [[maybe_unused]] int res;
      while ((res = Futex<uint64_t>::wait(0)) == -1 && errno != EAGAIN)
        ;
      // EAGAIN happens when the number of tokens was changed in the meantime.
      // We (supuriously?) woke up or failed to go to sleep because the number of tokens changed.
      // It is therefore not sure that there is a token for us. Refresh word and try again.
      word = m_word.load(make_load_order(success_order));
      Dout(dc::semaphore(res == 0), "Woke up! tokens = " << (word & tokens_mask) << "; waiters = " << (word >> nwaiters_shift));
      // We woke up, try to again to get a token.
    }
    else
    {
      // (Try to) atomically grab a token and stop being a waiter.
      if (m_word.compare_exchange_weak(word, word - one_waiter - 1, success_order))
      {
        Dout(dc::semaphore, "Successfully obtained a token. Now " << (ntokens - 1) << " tokens and " << ((word - one_waiter) >> nwaiters_shift) << " waiters left.");
        break;
      }
      // word was changed, try again.
    }
  }
}

} // namespace utils::threading
