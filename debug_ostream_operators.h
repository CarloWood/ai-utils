// SPDX-FileCopyrightText: 2014, 2016-2019, 2022, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief This file contains the declaration of debug serializers.
 */

#pragma once

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
