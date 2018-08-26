#include <array>
#include <cmath>

namespace utils {

inline char* backwards_itoa_unsigned(char* p, unsigned long n, unsigned int base)
{
  static char const digit[36] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
    'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
    'u', 'v', 'w', 'x', 'y', 'z' };

  *p = 0;
  do
  {
    *--p = digit[n % base];
    n /= base;
  }
  while (n > 0);
  return p;
}

inline char* backwards_itoa_signed(char* p, long n, int base)
{
  unsigned long const mask = n >> (sizeof(long) * 8 - 1);        // All 1's when n < 0, all 0's otherwise.
  unsigned long abs_n = (n + mask) ^ mask;
  p = backwards_itoa_unsigned(p, abs_n, base);
  if (mask)
    *--p = '-';
  return p;
}

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

} // namespace utils
