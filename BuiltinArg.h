/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of BuiltinArg.
 *
 * @Copyright (C) 2019  Carlo Wood.
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
