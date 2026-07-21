// SPDX-FileCopyrightText: 2017-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class FuzzyBool.
 */

#include "sys.h"
#include "debug.h"
#include "FuzzyBool.h"

namespace utils {

void print_FuzzyBoolPOD_on(std::ostream& os, FuzzyBoolEnum val)
{
  switch(val)
  {
    case fuzzy_true:
      os << "fuzzy::True";
      break;
    case fuzzy_was_true:
      os << "fuzzy::WasTrue";
      break;
    case fuzzy_was_false:
      os << "fuzzy::WasFalse";
      break;
    case fuzzy_false:
      os << "fuzzy::False";
      break;
  }
}

} // namespace utils
