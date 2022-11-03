#pragma once

// Usage:
//
// A class that has a print_on method needs to have the following
// declared in its namespace:
//
//   using utils::has_print_on::operator<<;
//
// Credits for this brilliant idea go to Alipha from Libera/#C++.
//

#ifdef EXAMPLE_CODE

  #include "utils/has_print_on.h"

  namespace N {
  // This class defines a print_on method.
  using utils::has_print_on::operator<<;

  struct A {
    void print_on(std::ostream& os) const;
  };

  // Then, from any namespace,

  void foo(std::ostream& os, N::A const& a)
  {
    os << a;    // Calls `a.print_on(os)` and return os for the next <<.
    ...

#endif // EXAMPLE_CODE

#include <type_traits>
#include <iosfwd>

namespace utils {
namespace has_print_on {

template<typename T, typename = void>
constexpr bool has_print_on = false;

template<typename T>
constexpr bool has_print_on<T, std::void_t<decltype(std::declval<T>().print_on(std::declval<std::ostream&>()))>> = true;

// Catch-all operator<< for classes with a print_on method.
template<typename T>
inline std::enable_if_t<has_print_on<T const>, std::ostream&>
operator<<(std::ostream& os, T const& data)
{
  data.print_on(os);
  return os;
}

} // namespace has_print_on
} // namespace utils
