// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Implementation of malloc_size.
//
// Copyright (C) 2018  Carlo Wood.
//
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file is part of ai-utils.
//
// ai-utils is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ai-utils is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.

#include "sys.h"
#include "malloc_size.h"
#include "nearest_multiple_of_power_of_two.h"
#include <unistd.h>     // sysconf.

namespace utils {

size_t malloc_size(size_t min_size)
{
  size_t const page_size = sysconf(_SC_PAGE_SIZE);
  // This is how glibc 2.27 malloc works.
  size_t required_heap_space = min_size + CW_MALLOC_OVERHEAD;
  size_t chunk_size = (required_heap_space <= 0x1fff0) ? 0x10 : page_size;
  size_t actual_used_heap_space = nearest_multiple_of_power_of_two(required_heap_space, chunk_size);
  return actual_used_heap_space - CW_MALLOC_OVERHEAD;
}

} // namespace utils
