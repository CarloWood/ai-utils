/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of clz.
 *
 * @Copyright (C) 2018  Carlo Wood.
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
