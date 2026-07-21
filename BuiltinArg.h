// SPDX-FileCopyrightText: 2018-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of BuiltinArg.
 */

#pragma once

#include <type_traits>

namespace utils {

// __builtin functions often only support the following three types.
// If we provide a smaller type, then it will be promoted to unsigned int.
// This helper class allows us to detect the actually used type.

template<typename T>
struct BuiltinArg
{
  static_assert(std::is_unsigned<T>::value && std::is_integral<T>::value, "The provided argument must be an unsigned integral type.");
  using type = unsigned int;
};

template<>
struct BuiltinArg<unsigned long>
{
  using type = unsigned long;
};

template<>
struct BuiltinArg<unsigned long long>
{
  using type = unsigned long long;
};

} // namespace utils
