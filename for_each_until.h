// SPDX-FileCopyrightText: 2014, 2016-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of for_each_until.
 */

#pragma once

namespace utils {

template<class InputIterator, class Function>
bool for_each_until(InputIterator first, InputIterator last, Function& fn)
{
  while (first != last)
  {
    if (fn(*first))
    {
      return true;
    }
    ++first;
  }
  return false;
}

} // namespace utils
