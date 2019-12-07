/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of malloc_size.
 *
 * @Copyright (C) 2018  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
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

#include "sys.h"
#include "malloc_size.h"
#include "nearest_multiple_of_power_of_two.h"
#include "nearest_power_of_two.h"
#include <unistd.h>     // sysconf.

namespace utils {

static size_t const page_size = sysconf(_SC_PAGE_SIZE);   // It is assumed that this is a power of two.
static constexpr size_t minimum_heap_size = 32;

// It seems that glibc 2.27 malloc is pretty efficient with memory;
// it uses a multiple of 16 bytes (mostly for alignment reasons)
// and has an overhead of 8 bytes. Sizes larger than 128kB are
// sometimes allocated per PAGE_SIZE (4096 bytes) but not always.
//
// Nevertheless, for an efficient use of the heap it seems that it
// can't harm to only allocate either a multiple of the page size,
// or use a power of two for smaller sizes.
//
// Given a minimum number required bytes, this function returns
// a (possibly) larger size such that the used heap size will be
// just that: 32, 64, 128, 256, 512, 1024, 2048, or a multiple of
// 4096 bytes.
size_t malloc_size(size_t min_size)
{
  size_t required_heap_space = min_size + config::malloc_overhead_c;
  size_t actual_used_heap_space;
  if (required_heap_space <= minimum_heap_size)
    actual_used_heap_space = minimum_heap_size;
  else if (required_heap_space < page_size)
    actual_used_heap_space = nearest_power_of_two(required_heap_space);
  else
    actual_used_heap_space = nearest_multiple_of_power_of_two(required_heap_space, page_size);
  return actual_used_heap_space - config::malloc_overhead_c;
}

// Return the largest possible size less than or equal max_size
// such that malloc(size) == size. If this is not possible because
// max_size < minimum_heap_size - config::malloc_overhead_c, then return 0.
size_t max_malloc_size(size_t max_size)
{
  if (max_size < minimum_heap_size - config::malloc_overhead_c)
    return 0;
  size_t heap_size = nearest_multiple_of_power_of_two(max_size + config::malloc_overhead_c + 1, page_size) - page_size;
  if (heap_size == 0)
    heap_size = nearest_power_of_two(max_size + config::malloc_overhead_c + 1) / 2;
  return heap_size - config::malloc_overhead_c;
}

} // namespace utils
