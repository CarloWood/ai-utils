/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of deposit_bits and extract_bits.
 *
 * @Copyright (C) 2025  Carlo Wood.
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
#include <cstdint>

#ifdef __BMI2__
#include <immintrin.h>          // _pdep_u32, _pdep_u64 _pext_u32, _pext_u64
//#else
#include "utils/macros.h"
#endif

namespace utils {

namespace {

template<std::unsigned_integral T>
T _pdep(T value, T mask)
{
  return _pdep_u32(value, mask);
}

template<>
uint64_t _pdep<uint64_t>(uint64_t value, uint64_t mask)
{
  return _pdep_u64(value, mask);
}

template<std::unsigned_integral T>
T _pext(T value, T mask)
{
  return _pext_u32(value, mask);
}

template<>
uint64_t _pext<uint64_t>(uint64_t value, uint64_t mask)
{
  return _pext_u64(value, mask);
}

} // namespace

// Insert zeroes into `value` at the positions that `mask` has unset bits.
// For example,
//
//   value  = 0000ABCD
//   mask   = 01101001
//            ↓  ↓ ↓↓  ← inserted zeroes.
//   result = 0AB0C00D
//
//   At the moment of examining B (offset = 3):
//            00100000 ← out_bit
//            0ABCD000 ← value << offset
//   Thus
//            result |= (value << offset) & out_bit
//
//   At the moment of examining mask position 4:
//            00010000 ← out_bit
//   we don't care what happens to the corresponding output bit because that can be reset at the end.
//   However, offset needs to be increments by one:
//   Thus
//            offset += (mask & out_bit) == 0
//
template<std::unsigned_integral T1, std::unsigned_integral T2>
#ifdef __BMI2__
inline
#endif
auto deposit_bits(T1 value, T2 mask)
{
  // Use the smallest of the two types if they differ.
  using result_type = std::conditional_t<sizeof(T1) <= sizeof(T2), T1, T2>;
  result_type result;

#if CW_DEBUG
  if constexpr (sizeof(T1) != sizeof(T2))
  {
    using larger_type = std::conditional_t<(sizeof(T1) >= sizeof(T2)), T1, T2>;
    larger_type large_value = std::same_as<larger_type, T1> ? value : mask;
    // The smaller type defines the number of bits that are allowed to be used.
    ASSERT(static_cast<result_type>(large_value) == large_value);
  }
#endif

#ifdef __BMI2__
  result = _pdep(static_cast<result_type>(value), static_cast<result_type>(mask));
#else
  int offset = 0;
  result = 0;
  for (result_type out_bit = 1; AI_LIKELY(out_bit != 0); out_bit <<= 1)
  {
    result |= (value << offset) & out_bit;
    offset += (mask & out_bit) == 0;
  }
  result &= mask;
#endif

  return result;
}

// Remove bits from `value` at the positions that `mask` has unset bits.
// For example,
//
//   value  = ABCDEFGH
//            ↑  ↑ ↑↑  ← removed bits
//   mask   = 01101001
//   result = 0000BCEH
//
//   At the moment of examining E (offset = 2):
//            00001000 ← in_bit
//            000000E0 ← (value & in_bit) >> offset
//   Thus
//            result |= (value & in_bit) >> offset
//
//   At the moment of examining mask position 4:
//            00010000 ← in_bit
//            000000D0 ← (value & in_bit) >> offset
//   Therefore we need to reset 'D' (and all other to be removed bits) to zero in advance.
//
//   Also, offset needs to be increments by one:
//   Thus
//            offset += (mask & out_bit) == 0
//
template<std::unsigned_integral T1, std::unsigned_integral T2>
#ifdef __BMI2__
inline
#endif
auto extract_bits(T1 value, T2 mask)
{
  // Use the smallest of the two types if they differ.
  using result_type = std::conditional_t<sizeof(T1) <= sizeof(T2), T1, T2>;
  result_type result;

#if CW_DEBUG
  if constexpr (sizeof(T1) != sizeof(T2))
  {
    using larger_type = std::conditional_t<(sizeof(T1) >= sizeof(T2)), T1, T2>;
    larger_type large_value = std::same_as<larger_type, T1> ? value : mask;
    // The smaller type defines the number of bits that are allowed to be used.
    ASSERT(static_cast<result_type>(large_value) == large_value);
  }
#endif

#ifdef __BMI2__
  result = _pext(static_cast<result_type>(value), static_cast<result_type>(mask));
#else
  value &= mask;
  int offset = 0;
  result = 0;
  for (result_type in_bit = 1; AI_LIKELY(in_bit != 0); in_bit <<= 1)
  {
    result |= (value & in_bit) >> offset;
    offset += (mask & in_bit) == 0;
  }
#endif

  return result;
}

} // namespace utils
