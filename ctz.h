// SPDX-FileCopyrightText: 2018-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of ctz.
 */

#pragma once

#include <type_traits>
#include <concepts>

namespace utils {

namespace {

template<std::unsigned_integral T>
constexpr int _ctz(T n)
{
  return __builtin_ctz(n);
}

template<>
constexpr int _ctz<unsigned long>(unsigned long n)
{
  return __builtin_ctzl(n);
}

template<>
constexpr int _ctz<unsigned long long>(unsigned long long n)
{
  return __builtin_ctzll(n);
}

} // namespace

// Function utils::ctz(n)
//
// Returns the Count of Trailing Zeroes in n (the index of the least significant set bit).
// Undefined when n == 0.
//
template<std::unsigned_integral T>
constexpr int ctz(T n)
{
  return _ctz(n);
}

} // namespace utils
