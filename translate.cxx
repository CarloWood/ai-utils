/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of translate::getString.
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
