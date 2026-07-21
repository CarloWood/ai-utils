// SPDX-FileCopyrightText: 2014, 2016-2019, 2022, 2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of translate::getString.
 */

#include "sys.h"
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
