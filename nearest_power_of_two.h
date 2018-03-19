// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of nearest_power_of_two.
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

#include <cstdint>

namespace utils {

// Helper function for nearest_power_of_two.
template<int d, int m, typename T>
T constexpr max_shift_or(T n)
{
  return max_shift_or<2 * d, m>(n | n >> d);
}

// Specializations. Unfortunately, partial non-type specialization are not allowed.
template<> uint64_t constexpr max_shift_or<32, 32>(uint64_t n) { return n | (n >> 32); }
template<> uint32_t constexpr max_shift_or<16, 16>(uint32_t n) { return n | (n >> 16); }
#if __SIZEOF_INT__ > 4  // For 64-bit int's.
template<> int constexpr max_shift_or<32, 32>(int n) { return n | (n >> 32); }
#endif
template<> int constexpr max_shift_or<16, 16>(int n) { return n | (n >> 16); }
template<> int constexpr max_shift_or<8, 8>(int n) { return n | (n >> 8); }
template<> int constexpr max_shift_or<4, 4>(int n) { return n | (n >> 4); }

// Round the positive integer n up to the nearest power of 2.
//
// If n is zero than 0 is returned.
//
// If n is signed and less than zero, but larger than std::numeric_limits<T>::min(),
// then also 0 is returned; however if n equals std::numeric_limits<T>::min() then
// std::numeric_limits<T>::min() is returned.
//
// If n is larger than the largest power of 2 that fits in type T,
// then std::numeric_limits<T>::min() is returned.
//
// Therefore, if T is a signed int and the result is less than or equal zero, then there
// was an out of range error. While if T is unsigned it is possible to interpret a result
// of 0 as `2 to the power the number of bits in T` provided that n wasn't equal to 0
// (which is still an out of range error).
//
// This function can be used as a constexpr, but is also optimized for runtime calculations.
template<typename T>
T constexpr nearest_power_of_two(T const n)
{
  return max_shift_or<1, 4 * sizeof(T)>(n - 1) + 1;
}

} // namespace utils
