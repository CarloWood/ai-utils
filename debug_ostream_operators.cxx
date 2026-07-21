// SPDX-FileCopyrightText: 2014, 2016-2019, 2022-2023, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief This file contains the definition of debug serializers.
 */

#include "sys.h"        // Put this outside the #ifdef CWDEBUG .. #endif in order
                        // to force recompilation after the configuration changed.

#include "debug_ostream_operators.h"
#include "c_escape.h"
#include "translate.h"
#include "AIAlert.h"

#include <ostream>

namespace AIAlert {

/// For debugging purposes. Write a AIAlert::Error to @a os.
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
      if (line.is_function_name() || line.is_filename_line())
        os << ": ";
    }
    else
      os << translate::getString(line.getXmlDesc(), line.args());
  }
  return os;
}

} // namespace AIAlert

namespace utils {

std::ostream& operator<<(std::ostream& os, PrintCEscaped str)
{
  c_escape(os, str.data_);
  return os;
}

} // namespace utils
