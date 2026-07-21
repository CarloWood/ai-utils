// SPDX-FileCopyrightText: 2025 Carlo Wood
// SPDX-License-Identifier: MIT

#pragma once

#include <string>
#include <array>

namespace utils {

std::string to_digits_string(int val, std::array<char const*, 11> const& digits);
std::string to_subscript_string(int val);
std::string to_superscript_string(int val);

}  // namespace utils
