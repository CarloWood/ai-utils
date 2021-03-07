#pragma once

// Usage:
//
// #include "utils/has_print_on.h"
//
// template<typename T>
// std::enable_if_t<utils::has_print_on<T const>, std::ostream&>
// operator<<(std::ostream& os, T const& data)
// {
//   data.print_on(os);
//   return os;
// }

#include <type_traits>
#include <iosfwd>

namespace utils {
namespace {

template<typename T, typename = void>
constexpr bool has_print_on = false;

template<typename T>
constexpr bool has_print_on<T, std::void_t<decltype(std::declval<T>().print_on(std::declval<std::ostream&>()))>> = true;

} // namespace
} // namespace
