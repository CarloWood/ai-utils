// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of ctz.
//
// Copyright (C) 2019 Carlo Wood.
//
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file is part of ai-utils.
//
// ai-utils is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ai-utils is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <type_traits>

namespace utils {

namespace {

constexpr int _ctz(unsigned int n)
{
  return __builtin_ctz(n);
}

constexpr int _ctz(unsigned long n)
{
  return __builtin_ctzl(n);
}

constexpr int _ctz(unsigned long long n)
{
  return __builtin_ctzll(n);
}

} // namespace

// Function utils::ctz(n)
//
// Returns the Count of Trailing Zeroes in n.
// Undefined when n == 0.
//
template<typename T>
constexpr int ctz(T n)
{
  static_assert(std::is_same<T, unsigned int>::value || std::is_same<T, unsigned long>::value || std::is_same<T, unsigned long long>::value,
      "The type of the argument to ctz can only be unsigned int, unsigned long or unsigned long long.");
  return _ctz(n);
}

} // namespace utils
