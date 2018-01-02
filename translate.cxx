// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Implementation of translate::getString.
//
// Copyright (C) 2014, 2016  Carlo Wood.
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

#include "translate.h"

namespace translate {

std::string getString(std::string const& xmlDesc, format_map_t const& format_map)
{
  std::string result = xmlDesc;
  for (format_map_t::const_iterator iter = format_map.begin(); iter != format_map.end(); ++iter)
  {
    size_t start_pos = 0;
    while(1)
    {
      size_t pos = result.find(iter->first, start_pos);
      if (pos == std::string::npos)
	break;
      size_t len = iter->first.length();
      result.replace(pos, len, iter->second);
      start_pos = pos + iter->second.length();
    }
  }
  return result;
}

} // namespace translate
