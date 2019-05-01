// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of nearest_multiple_of_power_of_two.
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

#include "debug.h"
#include <type_traits>
#ifdef CWDEBUG
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
