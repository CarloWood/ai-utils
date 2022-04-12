#pragma once

#include <vector>
#include <array>
#include <cstdint>

// Ultra fast lookup in small sized container of 64-bit integer values using dynamic perfect hashing.
// See https://stackoverflow.com/questions/71811886/
//
// The input is a std::vector<uint64_t> keys
// The only thing that is tried to partition the keys into sets of roughly 62 keys
// is by one or two bits that are determined in a heuristic way.
//
// Lets say that we have 200 keys, then ceil(200 / 62) = 4 - which can be encoded
// with two bits. Certain failure happens with more than 4 * 64 = 256 keys.
// Likely failure happens around 4 * 62 = 248 or more keys, depending on the keys.
// The utility is really intended for a maximum of 100 keys are so (something that
// fits well in two sets of 62, or less than 124 keys).
//
// Let the two heuristically determined bits be b[0] and b[1] (presented as bit mask)
// then if a key & b[0] is false we continue with Set 0 or 2, depending on the
// value of key & b[1]. If only one key is needed then b[1] is set to 0.
//

namespace utils {

using Matrix64x64 = std::array<uint64_t, 64>;

class UltraHash
{
 private:
  static int constexpr tested_bits = 2;
  std::array<uint64_t, tested_bits> m_b{};                      // Initialized with zero means: only one set.
  std::array<std::array<uint64_t, 6>, 1 << tested_bits> m_sets; // Allow up to two to the power m_b.size() sets.

  int set_index(uint64_t key) const                             // Return the index into m_sets for the set to be used for this key.
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

  // Create set from up to 64 keys. If there are less keys, the remaining keys should be...
  // Returns true on success and false on failure.
  bool create_set(std::array<uint64_t, 6>& set, std::array<uint64_t, 64> const& keys);

 public:
  // Initialize m_b and m_sets to work with keys.
  // Returns the size of the lookup table required (values returned by `index` will be less than this value).
  // The returned value will always be less than or equal 256.
  int initialize(std::vector<uint64_t> const& keys);

  // Returns a unique integer for each key that can be used as index into a table.
  int index(uint64_t key) const
  {
    int si = set_index(key);
    std::array<uint64_t, 6> const& s = m_sets[si];
    int idx = si << 6;
    int m = 1;
    for (int i = 0; i < 6; ++i)
      idx |= __builtin_parity(s[i] ^ key) << i;
    return idx;
  }
};

} // namespace utils
