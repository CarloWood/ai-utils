// SPDX-FileCopyrightText: 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of reverse_bits.
 */

#pragma once

#include <type_traits>
#include <cstdint>

namespace utils {

namespace detail {

constexpr uint8_t reverse_bits(uint8_t n)
{
  // See http://graphics.stanford.edu/~seander/bithacks.html#ReverseByteWith64Bits
  n = ((n * 0x80200802ULL) & 0x0884422110ULL) * 0x0101010101ULL >> 32;
  return n;
}

constexpr uint16_t reverse_bits(uint16_t n)
{
  n = ((n >> 1) & 0x5555) | ((n & 0x5555) << 1);
  n = ((n >> 2) & 0x3333) | ((n & 0x3333) << 2);
  n = ((n >> 4) & 0x0F0F) | ((n & 0x0F0F) << 4);
  n = ( n >> 8          ) | ( n           << 8);
  return n;
}

constexpr uint32_t reverse_bits(uint32_t n)
{
  // See http://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
  n = ((n >> 1) & 0x55555555) | ((n & 0x55555555) << 1);
  n = ((n >> 2) & 0x33333333) | ((n & 0x33333333) << 2);
  n = ((n >> 4) & 0x0F0F0F0F) | ((n & 0x0F0F0F0F) << 4);
  n = ((n >> 8) & 0x00FF00FF) | ((n & 0x00FF00FF) << 8);
  n = ( n >> 16             ) | ( n               << 16);
  return n;
}

constexpr uint64_t reverse_bits(uint64_t n)
{
  n = ((n >> 1)  & 0x5555555555555555) | ((n & 0x5555555555555555) << 1);
  n = ((n >> 2)  & 0x3333333333333333) | ((n & 0x3333333333333333) << 2);
  n = ((n >> 4)  & 0x0F0F0F0F0F0F0F0F) | ((n & 0x0F0F0F0F0F0F0F0F) << 4);
  n = ((n >> 8)  & 0x00FF00FF00FF00FF) | ((n & 0x00FF00FF00FF00FF) << 8);
  n = ((n >> 16) & 0x0000FFFF0000FFFF) | ((n & 0x0000FFFF0000FFFF) << 16);
  n = ( n >> 32                      ) | ( n                       << 32);
  return n;
}

} // namespace detail

// Function utils::reverse_bits(n)
//
// Let N be the number of bits in n.
// Returns a value where bit n and bit N-1-n are swapped, for 0 <= n < N/2.
//
// For example,
//
// 0b11010001 --> 0b10001011
//
template<typename T>
constexpr T reverse_bits(T n)
{
  static_assert(std::is_unsigned_v<T> && sizeof(T) <= sizeof(uint64_t),
      "The type of the argument to reverse_bits can only be an unsigned integral of at most uint64_t.");
  return detail::reverse_bits(n);
}

} // namespace utils
