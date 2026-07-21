// SPDX-FileCopyrightText: 2014, 2016-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of is_power_of_two.
 */

#pragma once

#include <type_traits>

// Usage:
//
// ASSERT(is_power_of_two(bit));
//
namespace utils {

template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
inline bool constexpr is_power_of_two(T n)
{
  return n > 0 && ((n - 1) & n) == 0;
}

} // namespace utils
