/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of backwards_itoa_unsigned, backwards_itoa_signed and template function itoa.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of ai-utils.
 *
 * Ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <array>
#include <cmath>
#include "constexpr_ceil.h"

namespace utils {

char* backwards_itoa_unsigned(char* p, unsigned long n, unsigned int base);
char* backwards_itoa_signed(char* p, long n, int base);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

template<typename T, size_t sz>
[[gnu::always_inline]] char const* itoa(std::array<char, sz>& buf, T n);

template<typename T, size_t sz>
char const* itoa(std::array<char, sz>& buf, T n)
{
  static constexpr int is_signed = std::is_signed<T>::value ? 1 : 0;
  // 2.40824 == ln(256) / ln(10), converting from byte length to digits.
  static_assert(sz > constexpr_ceil(2.40824 * (sizeof(T) - 0.125 * is_signed)) + is_signed,
      "The size of the array is not large enough to hold the maximum value of T.");
  if constexpr (is_signed)
    return backwards_itoa_signed(&buf[sz], n, 10);
  else
    return backwards_itoa_unsigned(&buf[sz], n, 10);
}

#pragma GCC diagnostic pop

} // namespace utils
