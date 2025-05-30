#pragma once

#include "config.h"

#ifndef USE_ENCHANTUM
#error "Please add the enchantum submodule to the root of the project."
#else

#include "enchantum/enchantum.hpp"

namespace utils {

template<typename E>
requires
  std::is_enum_v<E>
std::string_view to_string(E e)
{
  return enchantum::to_string(e);
}

} // namespace utils

#endif // USE_ENCHANTUM
