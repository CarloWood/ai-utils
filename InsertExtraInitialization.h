#pragma once

#include <boost/preprocessor/cat.hpp>

// This macro can be used to execute code as part of existing base constructors
// (after the base constructors, but BEFORE the body of any constructors of
// the class itself; so it is not recommended to have other constructors when
// using this).
//
// This is especially handy when using base class constructors while more
// initialization is needed.
//
// Usage:
//
#ifdef EXAMPLE_CODE
class Base;
[...]

class Derived : public Base
{
  using Base::Base;             // These are normally the only constructors (see below).

  [...]

  INSERT_EXTRA_INITIALIZATION
  {
    Dout(dc::notice, "WE GET HERE");
  };
};
#endif
//
// Note the semicolon after the closing brace of INSERT_EXTRA_INITIALIZATION.
//
// INSERT_EXTRA_INITIALIZATION *must* go after the last element of the class.
// Otherwise the default construction of following members will happen
// after the code of the INSERT_EXTRA_INITIALIZATION, which might have unexpected
// results when you use it to initialize said members.
//
// Note: when adding a constructor to Derived, its body will be executed
// after the body of INSERT_EXTRA_INITIALIZATION, just like it is relative
// to the construction of class members.

namespace utils {

  struct InsertExtraInitialization
  {
    InsertExtraInitialization operator+(InsertExtraInitialization, auto extra_init)
    {
      extra_init();
      return {};
    }
  };

} // namespace utils

#define INSERT_EXTRA_INITIALIZATION \
  [[no_unique_address]] ::utils::InsertExtraInitialization BOOST_PP_CAT(extra_init_, _LINE__) = ::utils::InsertExtraInitialization{} + [this]
