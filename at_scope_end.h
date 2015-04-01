/**
 * \file at_scope_end.h
 * \brief Definition of at_scope_end().
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
// auto&& f = at_scope_end(/* lambda expression to be executed when leaving scope */ });
//
// Note: in most cases using just 'auto' instead of a universal reference
// will generate the same code (and will always work), but some compilers
// might create a temporary AtScopeEnd that can be avoided by using the
// 'auto&&'.
//
// Also see EXAMPLE_CODE at the end of the file.

#ifndef UTILS_AT_SCOPE_END_H
#define UTILS_AT_SCOPE_END_H

#include <functional>

namespace utils {

// This class is not thread-safe and only intended to be accessed
// by a single thread, the one that constructed it.

template<typename T>
class AtScopeEnd
{
  private:
    T m_lambda;
    bool m_execute_at_destruction;

  public:
    AtScopeEnd(T const& t) : m_lambda(t), m_execute_at_destruction(true) { }
    ~AtScopeEnd() { once(); }
    AtScopeEnd(AtScopeEnd&& obj) : m_lambda(obj.m_lambda), m_execute_at_destruction(true) { obj.m_execute_at_destruction = false; }

    // Call this to execute lambda now, prior to the end of scope.
    // After calling this function, lambda will no longer be executed when this object is destructed.
    void now() { m_lambda(); m_execute_at_destruction = false; }

    // Execute lambda an extra.
    // Lambda will still (also) be executed at destruction (if execute() wasn't called before).
    void extra() { m_lambda(); }

    // Execute lambda iff neither now() nor once() were called before.
    void once() { if (m_execute_at_destruction) m_lambda(); m_execute_at_destruction = false; }
};

} // namespace utils

template<typename T>
utils::AtScopeEnd<T> at_scope_end(T lambda)
{
  return {lambda};
}

#endif // UTILS_AT_SCOPE_END_H


#ifdef EXAMPLE_CODE
// Compile as: g++ -std=c++11 -DEXAMPLE_CODE -x c++ AtScopeEnd.h

#include <cassert>

int main()
{
  int n = 0;
  try
  {
    ++n;
    // Execute { --n; } when leaving this scope, in an exception safe manner.
    auto&& decrement_n = at_scope_end([&n]{ --n; });
    assert(n == 1);
    throw std::exception();
    // If we would not throw, we could do for example:
    decrement_n.once();
    assert(n == 0);
  }
  catch(std::exception const&)
  {
  }
  assert(n == 0);
}

#endif // EXAMPLE_CODE
