// SPDX-FileCopyrightText: 2014, 2016-2019, 2022, 2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of double_to_str_precision.
 */

#include "sys.h"
#include "double_to_str_precision.h"

#include <sstream>
#include <iomanip>

std::string double_to_str_precision(double d, int min, int max)
{
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(max) << d;
  std::string str = oss.str();
  int dot = str.find_first_of(".");
  int pos = str.find_last_of("123456789");
  if (dot != (int)std::string::npos)
  {
    int precision = std::max(min, (pos != (int)std::string::npos) ? pos - dot : 0);
    str = str.substr(0, dot + ((precision > 0) ? 1 + precision : 0));
  }
  return str;
}
