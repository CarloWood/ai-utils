/**
 * @file
 * @brief Definition of ulong_to_base.
 *
 * Copyright (C) 2017 Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <string>

// Usage:
//
// std::cout << ulong_to_base(n, "abcdefghijklmnopqrstuvwxyz") << std::endl;
//
// Converts 'n' to base 26 where 'a' = 0, 'b' = 1, ... 'z' = 25.

template<int base_plus_one>
std::string ulong_to_base(unsigned long n, char const (&digits) [base_plus_one])
{
  int constexpr base = base_plus_one - 1;
  char buf[8 * sizeof(n)];
  std::string::size_type const end = sizeof(buf);
  std::string::size_type count = 0;
  do buf[end - ++count] = digits[n % base]; while ((n /= base));
  return std::string(buf + end - count, count);
}
