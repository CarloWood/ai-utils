// SPDX-FileCopyrightText: 2026 Carlo Wood
// SPDX-License-Identifier: MIT

#pragma once

namespace utils {

struct PrintWStringView {
  std::wstring_view text;
};

PrintWStringView print_wstring(std::wstring_view text)
{
  return {text};
}

std::ostream& operator<<(std::ostream& os, PrintWStringView value)
{
  for (wchar_t wc : value.text)
  {
    auto const codepoint = static_cast<std::uint32_t>(wc);

    if (codepoint <= 0x7f)
      os.put(static_cast<char>(codepoint));
    else if (codepoint <= 0x7ff)
    {
      os.put(static_cast<char>(0xc0 | (codepoint >> 6)));
      os.put(static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    else if (codepoint <= 0xffff)
    {
      os.put(static_cast<char>(0xe0 | (codepoint >> 12)));
      os.put(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
      os.put(static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    else if (codepoint <= 0x10ffff)
    {
      os.put(static_cast<char>(0xf0 | (codepoint >> 18)));
      os.put(static_cast<char>(0x80 | ((codepoint >> 12) & 0x3f)));
      os.put(static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
      os.put(static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    else
      os << "\xef\xbf\xbd";  // Emit U+FFFD for an invalid code point.
  }

  return os;
}

} // namespace utils
