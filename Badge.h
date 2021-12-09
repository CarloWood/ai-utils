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

} // namespace utils
