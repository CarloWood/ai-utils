#pragma once

// Usage:
//
// A class that has a to_string method needs to have the following
// declared in its namespace:
//
//   using utils::has_to_string::to_string;
//

#ifdef EXAMPLE_CODE

  #include "utils/to_string.h"

  namespace N {
  // This class defines a to_string method.
  using utils::has_to_string::to_string;

  struct A {
    std::string to_string() const;
  };

  // Then, from any namespace,

  void foo(N::A const& a)
  {
    auto s = to_string(a); // Calls `a.to_string()`.
    ...


  // And or

  namespace N {
  // This class defines a to_string method.
  using utils::has_to_string::to_string;

  struct C;
  struct B {
    static std::string to_string(C const& c) const;
  };

  } // namespace N

  // Must specialize enclosing_class
  template<>
  class enclosing_class<C>
  {
    using type = B;
  };

  // Then, from any namespace,

  void foo(N::C const& c)
  {
    auto s = to_string(c); // Calls `N::B::to_string(c)` (note that C must be defined in N).
    ...

#endif // EXAMPLE_CODE

#include <type_traits>
#include <iosfwd>
#include <concepts>
#include <string>

namespace utils::has_to_string {

// Must have a non-static member function `to_string()` with a return type that is convertible to std::string.
template<typename T>
concept ConceptHasToString = requires(T a)
{
  { a.to_string() } -> std::convertible_to<std::string>;
};

// Call member function T::to_string when using to_string(T const&).
template<ConceptHasToString T>
std::string to_string(T const& data)
{
  return data.to_string();
}

// Must have a static member function `to_string(E const&)` with a return type that is convertible to std::string.
template<typename T, typename E>
concept ConceptHasStaticToString =
    requires(E arg) {
      { T::to_string(arg) } -> std::convertible_to<std::string>;
    };

// Must be specialized to convert from E to T.
template<typename E>
struct enclosing_class
{
};

// Call static member function T::to_string(E const&) when using to_string(E const&),
// where enclosing_class<E>::type must be specialized as T.
template<typename E, typename T = typename enclosing_class<E>::type>
requires ConceptHasStaticToString<T, E>
std::string to_string(E e)
{
  return T::to_string(e);
}

} // namespace utils::has_to_strings
