// SPDX-FileCopyrightText: 2018-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of clz.
 */

#pragma once

#include <type_traits>

namespace utils {

namespace {

constexpr int _clz(unsigned int n)
{
  return __builtin_clz(n);
}

constexpr int _clz(unsigned long n)
{
  return __builtin_clzl(n);
}

constexpr int _clz(unsigned long long n)
{
  return __builtin_clzll(n);
}

} // namespace

// Function utils::clz(n)
//
// Returns the Count of Leading Zeroes in n.
// Undefined when n == 0.
//
template<typename T>
constexpr int clz(T n)
{
  static_assert(std::is_same<T, unsigned int>::value || std::is_same<T, unsigned long>::value || std::is_same<T, unsigned long long>::value,
      "The type of the argument to clz can only be unsigned int, unsigned long or unsigned long long.");
  return _clz(n);
}

} // namespace utils
