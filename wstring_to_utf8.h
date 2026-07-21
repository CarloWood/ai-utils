// SPDX-FileCopyrightText: 2026 Carlo Wood
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace utils {

/**
 * @brief Convert a sequence of Unicode code points stored as `wchar_t` to UTF-8.
 *
 * This function is intended for converting `std::wstring` values on platforms
 * where `wchar_t` is a 32-bit Unicode code point, at boundaries that require
 * UTF-8, such as writing them to a narrow output stream. Each `wchar_t` is
 * interpreted as one Unicode code point, independently of the current locale.
 *
 * @param text The sequence of Unicode code points to convert.
 * @return A `std::string` containing the corresponding UTF-8 byte sequence.
 *
 * @throws std::range_error If `text` contains a surrogate value or a code point
 * outside the Unicode range.
 *
 * This implementation requires 32-bit `wchar_t`, as used on Linux, and therefore
 * does not support platforms where `std::wstring` contains UTF-16 code units.
 * It performs encoding only: it does not normalize the text, validate grapheme
 * structure, or remove embedded null characters.
 */
std::string wstring_to_utf8(std::wstring_view text)
{
  static_assert(sizeof(wchar_t) == 4, "to_utf8 requires a platform with 32-bit wchar_t.");

  std::string result;
  result.reserve(text.size());

  for (wchar_t wc : text)
  {
    std::uint32_t const cp = static_cast<std::uint32_t>(wc);

    if (cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
      throw std::range_error("Invalid Unicode code point in std::wstring.");

    if (cp <= 0x7f)
    {
      result.push_back(static_cast<char>(cp));
    }
    else if (cp <= 0x7ff)
    {
      result.push_back(static_cast<char>(0xc0 | (cp >> 6)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
    else if (cp <= 0xffff)
    {
      result.push_back(static_cast<char>(0xe0 | (cp >> 12)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
    else
    {
      result.push_back(static_cast<char>(0xf0 | (cp >> 18)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3f)));
      result.push_back(static_cast<char>(0x80 | (cp & 0x3f)));
    }
  }

  return result;
}

} // namespace utils
