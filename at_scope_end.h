// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of @c at_scope_end.
//
// Copyright (C) 2015 - 2017  Carlo Wood.
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

// Usage:
//
// auto&& f = at_scope_end(/* lambda expression to be executed when leaving scope */);
//
// Note: in most cases using just 'auto' instead of a universal reference
// will generate the same code (and will always work), but some compilers
// might create a temporary AtScopeEnd that can be avoided by using the '&&'.
//
// Also see EXAMPLE_CODE at the end of the file.

#pragma once

#include <functional>

namespace utils {

// This class is not thread-safe and only intended to be accessed
// by a single thread, the one that constructed it.

template<typename T>
class AtScopeEnd
{
  private:
    T m_lambda;
    mutable bool m_executed;

  public:
    AtScopeEnd(T const& t) : m_lambda(t), m_executed(false) { }
    ~AtScopeEnd() { once(); }
    AtScopeEnd(AtScopeEnd const& obj) : m_lambda(obj.m_lambda), m_executed(false) { }
    AtScopeEnd(AtScopeEnd&& obj) : m_lambda(obj.m_lambda), m_executed(false)
    {
      obj.m_executed = true;     // Prevent execution from the destructor of the moved temporary 'obj'.
    }

    // Call this to execute lambda now, prior to the end of scope.
    // After calling this function, lambda will no longer be executed when this object is destructed.
    void now() const { m_lambda(); m_executed = true; }

    // Execute lambda iff neither now() nor once() were called before.
    void once() const { if (!m_executed) m_lambda(); m_executed = true; }

    // Execute lambda an additional time, regardless of calls to now() or once().
    void extra() const { m_lambda(); }
};

} // namespace utils

template<typename T>
utils::AtScopeEnd<T> at_scope_end(T lambda)
{
  return {lambda};
}

#ifdef EXAMPLE_CODE
// Compile as: g++ -std=c++11 -DEXAMPLE_CODE -x c++ at_scope_end.h

#include <cassert>

int main()
{
  int n;
  try
  {
    n = 2;
    // Execute { --n; } when leaving this scope, in an exception safe manner.
    auto&& decrement_n = at_scope_end([&n]{ --n; });
    assert(n == 2);
    decrement_n.extra(); // Just decrement n regardless.
    assert(n == 1);
    {
      // This copies the lambda, not its internal state with
      // regard to calls to now() and/or once().
      auto decrement_n_once_more(decrement_n);
      assert(n == 1);
      decrement_n_once_more.once();
      assert(n == 0);
    }
    assert(n == 0);
    ++n;
    throw std::exception();     // n is decremented at destruction of decrement_n.
    // If we would not throw, we could do for example:
    decrement_n.now();
    assert(n == 0);
    // n is not decremented at destruction of decrement_n
    // because we already called now().
  }
  catch(std::exception const&)
  {
  }
  assert(n == 0);
}

#endif // EXAMPLE_CODE
