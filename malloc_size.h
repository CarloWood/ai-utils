// SPDX-FileCopyrightText: 2014, 2016-2019, 2022 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of malloc_size.
 */

#pragma once

#include <cstddef>      // size_t

namespace utils {

size_t malloc_size(size_t min_size);
size_t max_malloc_size(size_t max_size);

} // namespace utils
