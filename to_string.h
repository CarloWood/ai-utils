#pragma once

// Adds support for print enums by ADL.
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
// An enum defined in a namespace (including global namespace) that wants to
// use `to_string(e)` must add the following inside its namespace in order for
// ADL to work:
//
#ifdef EXAMPLE_CODE

  #include "utils/to_string.h"

  namespace N {
  // In order to use to_string with E.
  using utils::enums::to_string;

  enum E {
    val1,
    val2 = 10,
    ...
  };

  } // namespace N

  // Then, from any namespace,

  void foo(N::E e)
  {
    std::string_view sv = to_string(e);
    ...

#endif // EXAMPLE_CODE

#include "config.h"

#ifndef USE_ENCHANTUM
#error "Please add the enchantum submodule to the root of the project."
#else

#include "enchantum/enchantum.hpp"

namespace utils::enums {

template<typename E>
requires std::is_enum_v<E>
inline std::string_view to_string(E e)
{
  return enchantum::to_string(e);
}

} // namespace utils::enums

#endif // USE_ENCHANTUM
