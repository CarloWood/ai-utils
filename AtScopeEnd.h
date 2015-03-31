/**
 * \file AtScopeEnd.h
 * \brief Declaration of AtScopeEnd.
 *
 * Copyright (C) 2015 Aleric Inglewood.
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

// Usage:
//
// AtScopeEnd temp([]{ /* code to be executed when leaving scope */ });
// See EXAMPLE_CODE at the end of the file.
//

#ifndef UTILS_AT_SCOPE_END_H
#define UTILS_AT_SCOPE_END_H

#include <functional>

namespace utils {

class AtScopeEnd {
  private:
    std::function<void()> m_lambda;
  public:
    AtScopeEnd(std::function<void()> lambda) : m_lambda(lambda) { }
    ~AtScopeEnd() { m_lambda(); }
};

} // namespace utils

#endif // UTILS_AT_SCOPE_END_H


#ifdef EXAMPLE_CODE
// Compile as: g++ -std=c++11 -DEXAMPLE_CODE -x c++ AtScopeEnd.h

#include <cassert>

int main()
{
  int n = 0;
  try
  {
    using namespace utils;
    // Execute { --n; } when leaving this scope, in an exception safe manner.
    ++n;
    AtScopeEnd __at_scope_end([&n]{ --n; });
    assert(n == 1);
    throw std::exception();
  }
  catch(std::exception const&)
  {
  }
  assert(n == 0);
}

#endif // EXAMPLE_CODE
