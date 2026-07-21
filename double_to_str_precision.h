// SPDX-FileCopyrightText: 2014, 2016-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of double_to_str_precision.
 */

#pragma once

#include <string>

/// Convert a double to a string with a minimal and maximal precision.
std::string double_to_str_precision(double d, int min, int max);
