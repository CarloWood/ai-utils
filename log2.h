// SPDX-FileCopyrightText: 2018-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of log2 and ceil_log2.
 */

#pragma once

#include "clz.h"
#include "BuiltinArg.h"

namespace utils {

// Function utils::log2(n)
//
// Returns, if
//
//   n == 0:    -1 (this is used by ceil_log2)
//   n > 0:     floor(log2(n)) (the index of the most significant set bit, aka 1 << log2(n) == n iff n is a power of 2).
//
template<typename T>
constexpr int log2(T n)
{
  return n == 0 ? -1 : 8 * sizeof(typename BuiltinArg<T>::type) - 1 - clz(static_cast<typename BuiltinArg<T>::type>(n));
}

// Function utils::ceil_log2(n)
//
// Returns ceil(log2(n)).
// Undefined if n == 0;
//
template<typename T>
constexpr int ceil_log2(T n)
{
  return 1 + log2(static_cast<T>(n - 1));
}

} // namespace utils
