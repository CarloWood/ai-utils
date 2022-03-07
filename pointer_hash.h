#pragma once

#include <cstdint>
#include <bit>

namespace utils {

// To add more pointers to hash h1.
inline uint64_t pointer_hash_combine(uint64_t h1, void* p2)
{
  // Magic numbers.
  static constexpr uint64_t m1 = 0x73b7b5e0bd014e8d;    // uint64_t h = 1; for (int n = 0; n < 5; ++n) h ^= h * 0x200000008000208c; cout << hex << h;
  static constexpr uint64_t m2 = 0x9e3779b97f4a7c15;    // https://www.wolframalpha.com/input?i=floor%282%5E64+%2F+golden+ratio%29+in+base+16

  uint64_t i2 = reinterpret_cast<uint64_t>(p2);
  // Spread low bits over full range of 64bits and combine the two, putting low bits over high bits.
  return (h1 * m1) ^ std::rotl((i2 << 6) + (i2 >> 2) * m2, 32);
}

// This function is suited to calculate a 64-bit hash from two 64-bit pointers to (heap allocated) memory.
inline uint64_t pointer_hash(void* p1, void* p2)
{
  return pointer_hash_combine(reinterpret_cast<uint64_t>(p1), p2);
}

} // namespace utils
