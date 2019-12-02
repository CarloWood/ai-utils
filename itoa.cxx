/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of backwards_itoa_unsigned and backwards_itoa_signed.
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

#include "sys.h"
#include "itoa.h"

namespace utils {

char* backwards_itoa_unsigned(char* p, unsigned long n, unsigned int base)
{
  static char const digit[36] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z' };

  *p = 0;
  do
  {
    *--p = digit[n % base];
    n /= base;
  }
  while (n > 0);
  return p;
}

char* backwards_itoa_signed(char* p, long n, int base)
{
  unsigned long const mask = n >> (sizeof(long) * 8 - 1);        // All 1's when n < 0, all 0's otherwise.
  unsigned long abs_n = (n + mask) ^ mask;
  p = backwards_itoa_unsigned(p, abs_n, base);
  if (mask)
    *--p = '-';
  return p;
}

} // namespace utils
