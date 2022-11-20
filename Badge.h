#pragma once

#include "get_Nth_type.h"
#include <cstddef>

namespace utils {

// See https://awesomekling.github.io/Serenity-C++-patterns-The-Badge
template<typename... types>
class Badge
{
  static constexpr std::size_t S = sizeof...(types);
  static_assert(S <= 4, "The maximum number of arguments to Badge is four.");
  friend typename get_Nth_type<0 % S, types...>::type;
  friend typename get_Nth_type<1 % S, types...>::type;
  friend typename get_Nth_type<2 % S, types...>::type;
  friend typename get_Nth_type<3 % S, types...>::type;

  Badge() {}
};

// Useful for passing the this pointer of the calling class.
//
// Usage:
//
// void A::f(utils::BadgeCaller<B> b)   // Can only be called by a member function of B.
// {
//   B* bp = b;                         // Pointer to the calling object.
//   ...
// }
//
// void B::f()
// {
//   a.f(this);                         // Pass this pointer of B from within a member function of B.
// }
//
// If only const access to B is allowed use:
//
// void A::f(utils::BadgeCaller<B> const b);
//
template<typename Caller>
class BadgeCaller
{
 private:
  Caller* m_caller;

  friend Caller;
  BadgeCaller(Caller* caller) : m_caller(caller) { }

 public:
  operator Caller*() { return m_caller; }
  operator Caller const*() const { return m_caller; }
};

} // namespace utils
