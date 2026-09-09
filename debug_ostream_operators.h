// SPDX-FileCopyrightText: 2014, 2016-2019, 2022, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief This file contains the declaration of debug serializers.
 */

#pragma once

#include "utils/has_print_on.h"
#include <iosfwd>                       // std::ostream&
#include <string_view>

namespace AIAlert {

class Error;

// Seems that clang++ needs this to be defined here instead of global namespace for gtest to work.
std::ostream& operator<<(std::ostream& os, Error const& error);

} // namespace AIAlert

namespace utils {

struct PrintCEscaped
{
  std::string_view data_;
};

inline PrintCEscaped print_c_escaped(std::string_view str) { return {str}; }
inline PrintCEscaped print_c_escaped(char const* str) { return {str}; }

std::ostream& operator<<(std::ostream& os, PrintCEscaped str);

} // namespace utils

#ifdef CWDEBUG

#include <iostream>
#ifdef QT_CORE_LIB
#include <QString>
#endif

// This namespace is used in LIBCWD_USING_OSTREAM_PRELUDE.
// Put overloads for types in namespace std here, as defining them in namespace std is UB.
namespace libcwd::ostream_operators {

// Add support for printing std::u8string to debug output.
// Use a template accepting arguments that are convertible to std::u8string_view, unless
// the exact argument type supplies print_on and should use its corresponding operator<<.
template<typename T>
requires (std::convertible_to<T, std::u8string_view> && !utils::has_print_on::has_print_on<std::remove_cvref_t<T> const>)
std::ostream& operator<<(std::ostream& os, T const& utf8_sv)
{
  os.write("u8\"", 3);
  os.write(reinterpret_cast<char const*>(utf8_sv.data()), utf8_sv.length());
  os.write("\"", 1);
  return os;
}

#ifdef QT_CORE_LIB
// Add support for printing QString to debug output.
template <typename T>
requires (std::convertible_to<T, QString> && !std::convertible_to<T, std::string> && !utils::has_print_on::has_print_on<std::remove_cvref_t<T> const>)
std::ostream& operator<<(std::ostream& os, T const& qstring)
{
  return os << NAMESPACE_DEBUG::print_string(qstring.toUtf8().constData());
}
#endif

} // libcwd::debug_ostream_operators

#endif // CWDEBUG
