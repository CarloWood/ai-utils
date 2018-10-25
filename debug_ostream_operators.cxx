// ai-utils -- C++ Core utilities
//
//! @file
//! @brief This file contains the definition of debug serializers.
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

#include "sys.h"        // Put this outside the #ifdef CWDEBUG .. #endif in order
                        // to force recompilation after the configuration changed.

#include "translate.h"
#include "AIAlert.h"

#include <ostream>

//! For debugging purposes. Write a AIAlert::Error to \a os.
std::ostream& operator<<(std::ostream& os, AIAlert::Error const& error)
{
  os << "AIAlert: ";
  int lines = 0;
  for (auto& line : error.lines())
    if (!line.is_prefix())
      ++lines;
  char const* indent_str = "\n    ";
  if (lines > 1)
    os << indent_str;
  unsigned int suppress_mask = 0;
  for (auto& line : error.lines())
  {
    if (line.suppressed(suppress_mask))
      continue;
    if (lines > 1 && line.prepend_newline())   // Empty line.
      os << indent_str;
    if (line.is_prefix())
    {
      os << line.getXmlDesc();
      if (line.is_function_name())
        os << ": ";
    }
    else
      os << translate::getString(line.getXmlDesc(), line.args());
  }
  return os;
}
