/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of UltraHash.
 *
 * @Copyright (C) 2022  Carlo Wood.
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

#include "parity.h"
#include <vector>
#include <array>
#include <cstdint>

// Ultra fast lookup in small sized container of 64-bit integer values using dynamic perfect hashing.
// See https://stackoverflow.com/questions/71811886/
//
// Measured time of a call to UltraHash::index(uint64_t key): 56 clock cycles
// on one core of a AMD Ryzen 9 3950X 16-Core Processor (average over 1000000 calls).
// Lowest measurement: 38 clock cycles (average over 10 calls).
//
// The input is a std::vector<uint64_t> with keys that should be well-hashed (make
// use of all 64 bits).
//
// The only thing that is tried to partition the keys into sets of roughly 62 keys
// is by one or two bits that are determined in a heuristic way.
//
// Lets say that we have 200 keys, then ceil(200 / 62) = 4 - which can be encoded
// with two bits. Certain failure happens with more than 4 * 64 = 256 keys.
// Likely failure happens near 4 * 62 = 248, depending on the keys.
// The utility is really intended for a maximum of 100 keys are so (something that
// fits well in two sets of 62, or less than 124 keys). It will work well up till
// 200 keys though: if something is intended to work with 100 keys, it should work
// with double that too to give plenty of room.
//
// Let the two heuristically determined bits be m_b[0] and m_b[1] (presented as bit mask)
// then if a key & m_b[0] is false we continue with Set 0 or 2, depending on the
// value of key & m_b[1]. If only one key is needed then m_b[1] is set to 0.
//

namespace utils {

using Matrix64x64 = std::array<uint64_t, 64>;

class UltraHash
{
 private:
  static int constexpr max_test_bits = 2;
  std::array<uint64_t, max_test_bits> m_b{};                            // Initialized with zero means: only one set.
  std::array<std::array<uint64_t, 6>, 1 << max_test_bits> m_M_sets;     // Allow up to two to the power m_b.size() sets.

  int set_index(uint64_t key) const                                     // Return the index into m_M_sets for the set to be used for this key.
  {
    int si = 0;
    int s_bit = 1;
    for (uint64_t mask : m_b)
    {
      if ((key & mask))
        si |= s_bit;
      s_bit <<= 1;
    }
    return si;
  }

  // Create set from up to 64 keys (n).
  // Returns true on success and false on failure.
  // The array `keys` is corrupted in both cases.
  bool create_set(std::array<uint64_t, 6>& set, std::array<uint64_t, 64>& keys, int n);

 public:
  // Returns the size of the lookup table required (values returned by `index` will be less than this value).
  // The returned value will always be less than or equal 2^(6 + max_test_bits) (aka 256).
  int initialize(std::vector<uint64_t> const& keys);

  // Returns a unique integer for each key that can be used as index into a table.
  int index(uint64_t key) const
  {
    int si = set_index(key);
    std::array<uint64_t, 6> const& s = m_M_sets[si];
    int idx = si << 6;
    int m = 1;
    for (int i = 0; i < 6; ++i)
      idx |= parity(s[i] ^ key) << i;
    return idx;
  }
};

} // namespace utils
