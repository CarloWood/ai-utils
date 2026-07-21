// SPDX-FileCopyrightText: 2018-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of mssb.
 */

#pragma once

#include <type_traits>
#include <cstdint>

namespace utils {

namespace {

constexpr uint8_t extend_mssb(uint8_t n)
{
  //           n: 10000000
  n |= n >> 1; // 11000000
  n |= n >> 2; // 11110000
  n |= n >> 4; // 11111111
  return n;
}

constexpr uint16_t extend_mssb(uint16_t n)
{
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  return n;
}

constexpr uint32_t extend_mssb(uint32_t n)
{
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  return n;
}

constexpr uint64_t extend_mssb(uint64_t n)
{
  n |= n >> 1;
  n |= n >> 2;
  n |= n >> 4;
  n |= n >> 8;
  n |= n >> 16;
  n |= n >> 32;
  return n;
}

} // namespace

// Function utils::mssb(n)
//
// Returns a value with only the most significant set bit of n.
// If n == 0 however, return 1 (same as for 1).
//
template<typename T>
constexpr T mssb(T n)
{
  static_assert(std::is_unsigned<T>::value && sizeof(T) <= sizeof(uint64_t),
      "The type of the argument to mssb can only be an unsigned integral of at most uint64_t.");
  return extend_mssb(n >> 1) + 1;
}

} // namespace utils
