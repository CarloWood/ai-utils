#pragma once

#include <cstdint>
#include <utility>

namespace utils {

// A smallest unsigned integral type that contains at least `N` bits.
template<int N>
using uint_leastN_t =
  std::conditional_t<N <=  8, uint8_t,
  std::conditional_t<N <= 16, uint16_t,
  std::conditional_t<N <= 32, uint32_t,
  std::conditional_t<N <= 64, uint64_t,
      void>>>>;

} // namespace utils
