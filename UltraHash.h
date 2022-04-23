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
// Measured time of a call to UltraHash::index(uint64_t key): 70 clock cycles
// on one core of a AMD Ryzen 9 3950X 16-Core Processor (average over 1000000 calls).
//
// The input is a std::vector<uint64_t> with keys that should be well-hashed (make
// use of all 64 bits).
//
// The only thing that is tried is to partition the keys into sets of roughly 62 keys
// by one, two, three or four bits that are determined in a heuristic way.
//
// Lets say that we have 800 keys, then ceil(800 / 62) = 13 - which can be encoded
// with four bits. Certain failure happens with more than 16 * 64 = 1024 keys.
// Likely failure happens near 16 * 62 = 992, depending on the keys.
// The utility is really intended for a maximum of 800 keys are so (something that
// fits well in 16 sets of 62).
//
// For example, let the two heuristically determined bits be b0 = 1 << m_shift[0]
// and b1 = 1 << m_shift[1] then if a key has b0 unset we continue with Set 0 or 2,
// depending on whether key has b1 set or not.
//

namespace utils {

class UltraHash
{
 public:
  // If you REALLY need more than 800 keys, you can increase this a bit.
  // The main disadvantage is that the of UltraHash will double for each
  // bit; and eventually `initialize` might get very slow.
  static int constexpr max_test_bits = 4;                               // Return up till 4 + 6 = 10 bits (for use with 1024 sized lookup tables).
                                                                        // Which can savely be used to store at least up to 800 keys.
  static size_t constexpr brute_force_limit = 4096;                     // After this number of failed attempts with N bits, go to N+1 bits.
                                                                        // This avoids `initialize` to become extremely slow, at the cost of
                                                                        // perhaps requiring a lookup table that is twice as large as strictly
                                                                        // needed.

 private:
  int m_number_of_bits{};                                               // Default initialization causes set_index to always return 0 and therefore index()
                                                                        // to return values less than 64 (for, obviously, non-existent keys).
  std::array<int, max_test_bits> m_shift;
  std::array<std::array<uint64_t, 6>, 1 << max_test_bits> m_M_sets;     // Allow up to two to the power m_shift.size() sets.

  [[gnu::always_inline]] unsigned int set_index(uint64_t key) const     // Return the index into m_M_sets for the set to be used for this key.
  {
    unsigned int si = 0;
    for (int i = 0; i < m_number_of_bits; ++i)
      si |= ((key >> m_shift[i]) & 1) << i;
    return si;
  }

  // Create set from up to 64 keys (n).
  // Returns true on success and false on failure.
  // The array `keys` is corrupted in both cases.
  bool create_set(std::array<uint64_t, 6>& set, std::array<uint64_t, 64>& keys, int n);

 public:
  // Returns the size of the lookup table required (values returned by `index` will be less than this value).
  // The returned value will always be less than or equal 2^(6 + max_test_bits) (aka 256).
  //
  // This call takes typically a few milliseconds with outlayers up till 50 ms.
  int initialize(std::vector<uint64_t> const& keys);

  // Returns a unique integer for each key that can be used as index into a table.
  //
  // This call takes rougly 67 clock cycles (18 ns on a typical CPU).
  int index(uint64_t key) const
  {
    int si = set_index(key);
    std::array<uint64_t, 6> const& s = m_M_sets[si];
    int idx = si << 6;
    for (int i = 0; i < 6; ++i)
      idx |= parity(s[i] & key) << i;
    return idx;
  }
};

} // namespace utils
