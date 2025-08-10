#pragma once

#include <concepts>
#include <type_traits>

namespace utils {

// Return true if `std::min(a, b) < val <= std::max(a, b)`.
template<std::integral T>
[[gnu::always_inline]] inline bool is_between_lt_le(T a, T val, T b)
{
  using signed_t = std::make_signed_t<T>;
  return ((static_cast<signed_t>(a - val) ^ static_cast<signed_t>(b - val)) < 0);
}

// Return true if `std::min(a, b) <= val < std::max(a, b)`.
template<std::integral T>
[[gnu::always_inline]] inline bool is_between_le_lt(T a, T val, T b)
{
  using signed_t = std::make_signed_t<T>;
  return ((static_cast<signed_t>(val - a) ^ static_cast<signed_t>(val - b)) < 0);
}

} // namespace utils
