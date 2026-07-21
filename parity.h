// SPDX-FileCopyrightText: 2018-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of parity.
 */

#pragma once

#include <type_traits>

namespace utils {

namespace {

constexpr int _parity(unsigned int n)
{
  return __builtin_parity(n);
}

constexpr int _parity(unsigned long n)
{
  return __builtin_parityl(n);
}

constexpr int _parity(unsigned long long n)
{
  return __builtin_parityll(n);
}

} // namespace

// Function utils::parity(n)
//
// Returns the parity of set bits in n; if the number of set bits in n is even, returns 0; when odd, returns 1.
//
template<typename T>
constexpr int parity(T n)
{
  static_assert(std::is_same<T, unsigned int>::value || std::is_same<T, unsigned long>::value || std::is_same<T, unsigned long long>::value,
      "The type of the argument to ctz can only be unsigned int, unsigned long or unsigned long long.");
  return _parity(n);
}

} // namespace utils
