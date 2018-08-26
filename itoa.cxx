#include "sys.h"
#include "itoa.h"

namespace utils {

char* backwards_itoa_unsigned(char* p, unsigned long n, unsigned int base)
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

char* backwards_itoa_signed(char* p, long n, int base)
{
  unsigned long const mask = n >> (sizeof(long) * 8 - 1);        // All 1's when n < 0, all 0's otherwise.
  unsigned long abs_n = (n + mask) ^ mask;
  p = backwards_itoa_unsigned(p, abs_n, base);
  if (mask)
    *--p = '-';
  return p;
}

} // namespace utils
