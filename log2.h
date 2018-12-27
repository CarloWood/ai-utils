// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of clz, log2 and ceil_log2.
//
// Copyright (C) 2018 Carlo Wood.
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

template<typename T>
struct Clz
{
  static_assert(std::is_unsigned<T>::value && std::is_integral<T>::value, "The argument to clz/log2/ceil_log2 must be an unsigned integral type.");
  using type = unsigned int;
};

template<>
struct Clz<unsigned long>
{
  using type = unsigned long;
};

template<>
struct Clz<unsigned long long>
{
  using type = unsigned long long;
};

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

// Function utils::log2(n)
//
// Returns, if
//
//   n == 0:    -1 (this is used by CeilLog2)
//   n > 0:     floor(log2(n))
//
template<typename T>
constexpr int log2(T n)
{
  return n == 0 ? -1 : 8 * sizeof(typename Clz<T>::type) - 1 - clz(static_cast<typename Clz<T>::type>(n));
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
