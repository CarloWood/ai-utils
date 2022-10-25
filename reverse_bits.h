/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of reverse_bits.
 *
 * @Copyright (C) 2022  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
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
