/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of constexpr_ceil and constexpr_ceil.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
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
