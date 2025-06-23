/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief This file contains the declaration of debug serializers.
 *
 * @Copyright (C) 2014, 2016  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
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
