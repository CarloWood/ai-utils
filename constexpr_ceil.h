/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of constexpr_ceil and constexpr_ceil.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of ai-utils.
 *
 * Ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstdint>

// For use in static_assert.

namespace utils {

// Copied from https://stackoverflow.com/questions/31952237/looking-for-a-constexpr-ceil-function

constexpr int32_t constexpr_ceil(float num)
{
  return (static_cast<float>(static_cast<int32_t>(num)) == num)
      ? static_cast<int32_t>(num)
      : static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
}

constexpr int64_t constexpr_ceil(double num)
{
  return (static_cast<double>(static_cast<int64_t>(num)) == num)
      ? static_cast<int64_t>(num)
      : static_cast<int64_t>(num) + ((num > 0) ? 1 : 0);
}

} // namespace utils
