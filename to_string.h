#pragma once

#include "debug.h"

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
// Add the submodule enchantum to the root of the project with:
//   $ git submodule add https://github.com/ZXShady/enchantum.git
// and add:
//   # We use utils/to_string.h
//   add_subdirectory(enchantum)
// to the CMakeLists.txt file in the root of the project.
//
// If you already have the submodule enchantum then it is also
// possible that you forgot to add `AICxx::utils` as dependency
// to the target that the TU that results in this error is a part
// of (aka at AICxx::utils to the target_link_libraries of that target).
#error "Please add the enchantum submodule to the root of the project."
#else

#include "enchantum/enchantum.hpp"

namespace utils {

namespace adl_decls {
void to_string();

template<class T>
concept adl_to_stringable = requires(T t)
{
  to_string(t);
};
} // namespace adl_decls

template<typename T>
auto to_string(T const& t)
{
  if constexpr (requires { t.to_string(); })
    return t.to_string();
  else if constexpr (requires { T::to_string(t); })
    return T::to_string(t);
  else if constexpr (adl_decls::adl_to_stringable<T>)
    return to_string(t);
  else if constexpr (std::is_enum_v<T>)
    return enchantum::to_string(t);
  else
  {
    static_assert(false, "utils::string can not find a candidate");
    // Return something that can be written to an ostream, to avoid unnecessary compile errors.
    return std::string_view{""};
  }
}

} // namespace utils

#endif // USE_ENCHANTUM
