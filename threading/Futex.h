/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of Futex.
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

#include "utils/is_power_of_two.h"
#include <cstdint>
#include <cstddef>
#include <atomic>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/futex.h>
#include <endian.h>

namespace utils::threading {

template<typename T, int size_in_bytes = sizeof(T)>
class Futex
{
  static_assert(std::atomic<T>::is_always_lock_free, "std::atomic<T> must be lock free.");
  static_assert(sizeof(T) == sizeof(std::atomic<T>), "Sanity check failure!");  // Implied by the previous static_assert.
  static_assert(std::is_integral_v<T> && sizeof(T) % sizeof(int32_t) == 0, "T must be an integral type with a size that is a multiple of 4 bytes.");
  static_assert(utils::is_power_of_two(size_in_bytes) && size_in_bytes >= sizeof(T), "size_in_bytes must be a power of two that is greater than or equal to the size of T.");

  static constexpr int word_size_in_ints = sizeof(T) / sizeof(int32_t);
  static constexpr int padding_size_in_ints = size_in_bytes / sizeof(int32_t) - word_size_in_ints;
  static constexpr int index_32bits_word = (__BYTE_ORDER == __LITTLE_ENDIAN) ? 0 : word_size_in_ints - 1;

 private:
  [[gnu::always_inline]] int32_t* futex_word_ptr()
  {
    return &m_int_array[index_32bits_word];
  }

  int futex(int futex_op, uint32_t val, uint32_t val3)
  {
    return syscall(SYS_futex, futex_word_ptr(), futex_op, val, nullptr, nullptr, val3);
  }

  int futex(int futex_op, uint32_t val, uint32_t val2, Futex& futex2, uint32_t val3)
  {
    return syscall(SYS_futex, reinterpret_cast<int*>(&m_word), futex_op, val,
        reinterpret_cast<struct timespec const*>(static_cast<unsigned long>(val2)), futex2.futex_word_ptr(), val3);
  }

 protected:
  struct {
    union {
      std::atomic<T> m_word;
      int32_t m_int_array[word_size_in_ints];
    };
    int32_t m_padding[padding_size_in_ints];
  };

  Futex(T word) : m_word(word) { }

  int wait(uint32_t expected) noexcept
  {
    // This operation tests that the value at m_word still contains
    // the expected value, and if so, then sleeps waiting for a FUTEX_WAKE
    // operation on the futex word.
    //
    // The load of the value of the futex word is an atomic memory access
    // (i.e., using atomic machine instructions of the respective
    // architecture). This load, the comparison with the expected value,
    // and starting to sleep are performed atomically and totally ordered
    // with respect to other futex operations on the same futex word.
    //
    // If the thread starts to sleep, it is considered a waiter on this
    // futex word. If the futex value does not match expected, then the call
    // fails immediately with the value -1 and errno set to EAGAIN.
    //
    // Returns 0 if the caller was woken up.
    //
    // Note that a wake-up can also be caused by common futex usage patterns
    // in unrelated code that happened to have previously used the futex word's
    // memory location (e.g., typical futex-based implementations of Pthreads
    // mutexes can cause this under some conditions). Therefore, callers should
    // always conservatively assume that a return value of 0 can mean a
    // spurious wake-up, and use the futex word's value (i.e., the user-space
    // synchronization scheme) to decide whether to continue to block or not.
    return futex(FUTEX_WAIT_PRIVATE, expected, 0);
  }

  uint32_t wake(uint32_t n_threads) noexcept
  {
    // This operation wakes at most n_threads of the waiters that are
    // waiting (e.g., inside FUTEX_WAIT) on m_word.
    //
    // No guarantee is provided about which waiters are awoken (e.g.,
    // a waiter with a higher scheduling priority is not guaranteed
    // to be awoken in preference to a waiter with a lower priority).
    //
    // Returns the number of waiters that were woken up.
    return futex(FUTEX_WAKE_PRIVATE, n_threads, 0);
  }

  int32_t cmp_wake(uint32_t expected, uint32_t n_threads)
  {
    // See cmp_requeue. The idea is that this is the same as FUTEX_WAKE
    // but fails when m_word doesn't contain the value expected.
    return futex(FUTEX_CMP_REQUEUE_PRIVATE, n_threads, 0, *this, expected);
  }

  int32_t cmp_requeue(uint32_t expected, uint32_t n_threads, Futex& target, uint32_t target_limit) noexcept
  {
    // This operation first checks whether the location m_word still
    // contains the value expected. If not, the operation fails immediately
    // with the value -1 and errno set to error EAGAIN. Otherwise, the
    // operation wakes up a maximum of n_threads waiters that are waiting
    // on m_word. If there are more than n_threads waiters, then the
    // remaining waiters are removed from the wait queue of this futex and
    // added to the wait queue of the target futex. The target_limit argument
    // specifies an upper limit on the number of waiters that are requeued
    // to the futex target.
    //
    // Returns the total number of waiters that were woken up or requeued to target.
    // If this value is greater than n_threads, then the difference is the number
    // of waiters requeued to target.
    return futex(FUTEX_CMP_REQUEUE_PRIVATE, n_threads, target_limit, target, expected);
  }

  int32_t requeue(uint32_t n_threads, Futex& target, uint32_t target_limit) noexcept
  {
    // This operation performs the same task as FUTEX_CMP_REQUEUE except that no expected check is made.
    return futex(FUTEX_REQUEUE_PRIVATE, n_threads, target_limit, target, 0);
  }

  uint32_t wake_op(uint32_t n_threads, Futex& futex2, uint32_t n_threads2, uint32_t val3) noexcept
  {
    // The operation and comparison that are to be performed are encoded in the bits of the argument val3.
    // Pictorially, the encoding is:
    //
    //     val3:
    //     +---+---+-----------+-----------+                               Expressed in code, the encoding is:
    //     |op |cmp|   oparg   |  cmparg   |
    //     +---+---+-----------+-----------+                               #define FUTEX_OP(op, oparg, cmp, cmparg) \           .
    //       4   4       12          12    <== # of bits                       (((op & 0xf) << 28) | ((cmp & 0xf) << 24) | \    .
    //                                                                         ((oparg & 0xfff) << 12) | (cmparg & 0xfff))
    // The oparg and cmparg components are literal numeric values,
    // except as noted below.
    //
    // The FUTEX_WAKE_OP operation is equivalent to executing the
    // following code atomically and totally ordered with respect to
    // other futex operations on any of the two supplied futex words:
    //                                                                     op                        OPERATION
    //     int32_t prev_word2 = futex2.m_word.OPERATION(oparg);            FUTEX_OP_SET        0  // futex2.m_word.exchange(oparg);
    //     wake(n_threads);                                                FUTEX_OP_ADD        1  // futex2.m_word.fetch_add(oparg);
    //     if (prev_word2 CMP cmparg)                                      FUTEX_OP_OR         2  // futex2.m_word.fetch_or(oparg);
    //       futex2.wake(n_threads2);                                      FUTEX_OP_ANDN       3  // futex2.m_word.fetch_and(~oparg);
    //                                                                     FUTEX_OP_XOR        4  // futex2.m_word.fetch_xor(oparg);
    // In other words, FUTEX_WAKE_OP does the following:
    //                                                                     In addition, bitwise OR-ing the following value into op causes
    //  * Saves the original value of the futex2 word and performs an      (1 << oparg) to be used as the operand.
    //    operation to modify its value; this is an atomic
    //    read-modify-write memory access.                                     FUTEX_OP_ARG_SHIFT  8  // Use (1 << oparg) as operand.
    //
    //  * Wakes up a maximum of n_threads waiters on this futex; and       cmp                       CMP
    //                                                                     FUTEX_OP_CMP_EQ     0  // ==
    //  * Dependent on the results of a test of the original value of      FUTEX_OP_CMP_NE     1  // !=
    //    futex2, wakes up a maximum of n_threads2 waiters on futex2.      FUTEX_OP_CMP_LT     2  // <
    //                                                                     FUTEX_OP_CMP_LE     3  // <=
    //                                                                     FUTEX_OP_CMP_GT     4  // >
    //                                                                     FUTEX_OP_CMP_GE     5  // >=
    //
    // Returns the sum of the number of waiters woken on this futex plus the number of waiters woken on futex2.
    return futex(FUTEX_WAKE_OP_PRIVATE, n_threads, n_threads2, futex2, val3);
  }

  int wait_bitset(uint32_t expected, uint32_t bit_mask) noexcept
  {
    // This operation is like FUTEX_WAIT except that bit_mask is used to
    // provide a 32-bit bit mask to the kernel. This bit mask, in which at
    // least one bit must be set, is stored in the kernel-internal state of
    // the waiter. See the description of FUTEX_WAKE_BITSET for further details.
    //
    // Returns 0 if the caller was woken up. See FUTEX_WAIT for how to interpret
    // this correctly in practice.
    return futex(FUTEX_WAIT_BITSET_PRIVATE, expected, bit_mask);
  }

  uint32_t wake_bitset(uint32_t n_threads, uint32_t bit_mask) noexcept
  {
    // This operation is the same as FUTEX_WAKE except that bit_mask
    // is used to provide a 32-bit bit mask to the kernel.
    // This bit mask, in which at least one bit must be set, is used
    // to select which waiters should be woken up.  The selection is
    // done by a bit-wise AND of bit_mask and the bit mask which is
    // stored in the kernel-internal state of the waiter (the "wait"
    // bit_mask that is set using FUTEX_WAIT_BITSET). All of the waiters
    // for which the result of the AND is nonzero are woken up; the
    // remaining waiters are left sleeping.
    //
    // Returns the number of waiters that were woken up.
    return futex(FUTEX_WAKE_BITSET, n_threads, bit_mask);
  }
};

} // namespace utils::threading
