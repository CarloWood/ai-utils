/**
 * @file
 * @brief Implementation of double_to_str_precision.
 *
 * Copyright (C) 2014, 2016  Carlo Wood.
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

#include "double_to_str_precision.h"

#ifndef USE_PCH
#include <sstream>
#include <iomanip>
#endif

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
