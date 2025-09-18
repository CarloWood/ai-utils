/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of Semaphore.
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
