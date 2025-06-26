#pragma once

// Adds support for printing enums by ADL.
// For this to work you need to add https://github.com/ZXShady/enchantum.git
// (or the fork https://github.com/CarloWood/enchantum.git) to the root of
// your project:
//
// $ git submodule add https://github.com/ZXShady/enchantum.git
//
// And add `add_subdirectory(enchantum)` to the CMakeLists.txt file in the
// root of the project.
//
// Furthermore, every TU that includes this header must be linked with
// enchantum::enchantum, aka:
//
// target_link_libraries(executable
//   PRIVATE
//     ${AICXX_OBJECTS_LIST}
//     enchantum::enchantum
// )
//
// Usage:
//
#ifdef EXAMPLE_CODE

  #include "utils/to_string.h"

  namespace N {

  enum E {
    val1,
    val2 = 10,
    ...
  };

  } // namespace N

  // Then, from any namespace,

  void foo(N::E e)
  {
    std::string_view sv = utils::to_string(e);
    ...

  // This will call N::to_string(e) if that exists (found by ADL)
  // or else enchantum::to_string(e).

#endif // EXAMPLE_CODE

#include "config.h"

#ifndef USE_ENCHANTUM
#error "Please add the enchantum submodule to the root of the project."
#else

#include "enchantum/enchantum.hpp"

namespace utils {

namespace adl {
template<enchantum::Enum E>
constexpr auto to_string(E e) { return enchantum::to_string(e);}
} // namespace adl

struct {
  template<class E>
  requires std::is_enum_v<E>
  constexpr auto operator()(E e) const
  {
    using adl::to_string;
    // Uses ADL to find a to_string in the namespace of E, or else falls back to enchantum::to_string.
    return to_string(e);
  }
} inline constexpr to_string;

} // namespace utils

#endif // USE_ENCHANTUM
