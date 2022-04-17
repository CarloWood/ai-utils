/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of parity.
 *
 * @Copyright (C) 2022  Carlo Wood.
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
