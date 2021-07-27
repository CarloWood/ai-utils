#pragma once

#include <boost/preprocessor/cat.hpp>

// This macro can be used to insert a lamba as part of construction;
// it declares a data member of unspecified name and type, but size
// zero, whose initialization runs the given code.
//
// This is especially handy when inheriting base class constructors
// while extra initialization of the derived class is still needed.
//
// The lambda is NOT run for copy and move constructors.
//
// Intended usage:
//
#ifdef EXAMPLE_CODE
class Base;
[...]

class Derived : public Base
{
  using Base::Base;                     // Executed before.
  Derived(Derived const&) = delete;     // Unless no special code is needed for any of the data
                                        // members of Derived, it wouldn't be safe to allow copying.

  Derived(...);                         // Body is executed after.

  [...]

  MemberIsConstructedBefore x;          // Executed before.

  INSERT_EXTRA_INITIALIZATION
  {
    Dout(dc::notice, "WE GET HERE");
  };

  MemberIsConstructedAfter y;           // Executed after.
};
#endif
//
// Note the semicolon after the closing brace of INSERT_EXTRA_INITIALIZATION.
//
// INSERT_EXTRA_INITIALIZATION should go after the last element of the class.
// Otherwise the default construction of following members will happen after
// the lambda of the INSERT_EXTRA_INITIALIZATION, which might have unexpected
// results when you used it to alter said members.
//
// Note: when adding a constructor to Derived, its body will be executed
// after the lambda of INSERT_EXTRA_INITIALIZATION (which, afterall, is merely
// a member construction).

namespace utils {

  struct InsertExtraInitialization
  {
    InsertExtraInitialization operator+(auto extra_init)
    {
      extra_init();
      return {};
    }
  };

} // namespace utils

#define INSERT_EXTRA_INITIALIZATION \
  [[no_unique_address]] ::utils::InsertExtraInitialization BOOST_PP_CAT(extra_init_, __LINE__) = ::utils::InsertExtraInitialization{} + [this]
