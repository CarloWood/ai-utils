/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of ulong_to_base.
 *
 * @Copyright (C) 2017  Carlo Wood.
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

#include <string>

// Usage:
//
// std::cout << utils::ulong_to_base(n, "abcdefghijklmnopqrstuvwxyz") << std::endl;
//
// Converts 'n' to base 26 where 'a' = 0, 'b' = 1, ... 'z' = 25.

namespace utils {

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

} // namespace utils
