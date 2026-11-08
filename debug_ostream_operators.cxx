// SPDX-FileCopyrightText: 2014, 2016-2019, 2022-2023, 2025-2026 Carlo Wood
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

#include <ostream>

namespace utils {

std::ostream& operator<<(std::ostream& os, PrintCEscaped str)
{
  c_escape(os, str.data_);
  return os;
}

} // namespace utils
