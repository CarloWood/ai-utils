#include "sys.h"
#include "to_digits_string.h"
#include <cstring>
#include <limits>
#include <algorithm>

namespace utils {

std::string to_digits_string(int val, std::array<char const*, 11> const& digits)
{
  // This is the maximum length of each digit string.
  // The UTF8 encoding of subscript and superscript characters are at most 3 bytes long.
  constexpr size_t max_digit_length = 4;

  char buffer[(std::numeric_limits<int>::digits10 + 1) * max_digit_length]; // 1 extra for the sign.
  char* buf = buffer;

  if (val < 0)
  {
    val = -val;
    char const* digit_str = digits[10];
    buf = std::strncpy(buf, digit_str, max_digit_length) + std::min(max_digit_length, std::strlen(digit_str));
  }

  int digit = 1;
  unsigned int digit_count = 1;
  while (val >= digit * 10)
  {
    digit *= 10;
    ++digit_count;
  }

  for (; digit > 0; digit /= 10)
  {
    char const* digit_str = digits[val / digit % 10];
    buf = std::strncpy(buf, digit_str, max_digit_length) + std::min(max_digit_length, std::strlen(digit_str));
  }

  return {buffer, static_cast<size_t>(buf - buffer)};
}

std::string to_subscript_string(int val)
{
  static std::array<char const*, 11> subscript_digits = {
    "₀", "₁", "₂", "₃", "₄", "₅", "₆", "₇", "₈", "₉", "₋"
  };
  return to_digits_string(val, subscript_digits);
}

std::string to_superscript_string(int val)
{
  static std::array<char const*, 11> superscript_digits = {
    "⁰", "¹", "²", "³", "⁴", "⁵", "⁶", "⁷", "⁸", "⁹", "⁻"
  };
  return to_digits_string(val, superscript_digits);
}

}  // namespace utils
