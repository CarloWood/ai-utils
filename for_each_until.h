/**
 * \file for_each_until.h
 * \brief Declaration of for_each_until.
 *
 * Copyright (C) 2014 Aleric Inglewood.
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

#ifndef UTILS_FOR_EACH_UNTIL_H
#define UTILS_FOR_EACH_UNTIL_H

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

#endif // UTILS_FOR_EACH_UNTIL_H
