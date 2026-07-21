// SPDX-FileCopyrightText: 2018-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of popcount.
 */

#pragma once

#include <type_traits>

namespace utils {

namespace {

constexpr int _popcount(unsigned int n)
{
  return __builtin_popcount(n);
}

constexpr int _popcount(unsigned long n)
{
  return __builtin_popcountl(n);
}

constexpr int _popcount(unsigned long long n)
{
  return __builtin_popcountll(n);
}

} // namespace

// Function utils::popcount(n)
//
// Returns the Count of set bits in n.
//
template<typename T>
constexpr int popcount(T n)
{
  static_assert(std::is_same<T, unsigned int>::value || std::is_same<T, unsigned long>::value || std::is_same<T, unsigned long long>::value,
      "The type of the argument to popcount can only be unsigned int, unsigned long or unsigned long long.");
  return _popcount(n);
}

} // namespace utils
