// SPDX-FileCopyrightText: 2014, 2016-2019, 2022, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of nearest_multiple_of_power_of_two.
 */

#pragma once

#include "debug.h"
#include <type_traits>
#if CW_DEBUG
#include "is_power_of_two.h"
#endif

namespace utils {

// Returns the smallest possible value N * power_of_two that is greater than or equal to n.
//
// n must be a non-negative integer and
// power_of_two must be a (non-negative) integer power of two (2ᵏ, k ∈ ℕ).
// Hence, the allowed values of power_of_two are 1, 2, 4, 8, 16, ... etc.
//
// For example,
//
//      n       power_of_two    N       result
//      0       x               0       0
//      1       x               1       x
//      x       1               x       x
//      1       2               1       2
//      2       2               1       2
//      3       2               2       4
//      9       4               3      12
//      ...
//
template<typename T, std::enable_if_t<std::is_unsigned_v<T>, int> = 0>
T constexpr nearest_multiple_of_power_of_two(T const n, T const power_of_two)
{
  ASSERT(is_power_of_two(power_of_two));
  return (n + power_of_two - 1) & -power_of_two;
}

} // namespace utils
