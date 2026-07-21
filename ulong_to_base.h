// SPDX-FileCopyrightText: 2014, 2016-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of ulong_to_base.
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
