#pragma once

#include <array>
#include <cmath>

namespace utils {

char* backwards_itoa_unsigned(char* p, unsigned long n, unsigned int base);
char* backwards_itoa_signed(char* p, long n, int base);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

template<typename T, size_t sz>
char const* itoa(std::array<char, sz>& buf, T n) __attribute__ ((always_inline));

template<typename T, size_t sz>
char const* itoa(std::array<char, sz>& buf, T n)
{
  static constexpr int is_signed = std::is_signed<T>::value ? 1 : 0;
  // 2.40824 == ln(256) / ln(10), converting from byte length to digits.
  static_assert(sz > std::ceil(2.40824 * (sizeof(T) - 0.125 * is_signed)) + is_signed,
      "The size of the array is not large enough to hold the maximum value of T.");
  if (is_signed)
    return backwards_itoa_signed(&buf[sz], n, 10);
  else
    return backwards_itoa_unsigned(&buf[sz], n, 10);
}

#pragma GCC diagnostic pop

} // namespace utils
