/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of ctz.
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
