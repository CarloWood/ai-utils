// SPDX-FileCopyrightText: 2014, 2016-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of print_using.
 */

#include "sys.h"
#include "print_using.h"

namespace utils {

PrintUsing1 print_using(void (*print_on)(std::ostream&))
{
  return { print_on };
}

} // namespace utils
