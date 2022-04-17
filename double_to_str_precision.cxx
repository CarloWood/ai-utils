/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of double_to_str_precision.
 *
 * @Copyright (C) 2014, 2016  Carlo Wood.
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
