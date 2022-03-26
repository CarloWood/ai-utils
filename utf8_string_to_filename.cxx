#include "sys.h"
#include "utf8_string_to_filename.h"
#include "utf8_glyph_length.h"
#include "debug.h"
#include <vector>
#include <string_view>

namespace utils {
namespace detail::us2f {

static constexpr char8_t escape = '%';

std::u8string to_hex_string(char8_t c);

class Dictionary
{
 private:
  std::vector<std::u8string_view> m_words;

 public:
  Dictionary(std::u8string const&);

  size_t size() const { return m_words.size(); }
  void add(std::u8string_view glyph);
  int find(std::u8string_view glyph) const;
  std::u8string_view operator[](int index) const { return m_words[index]; }
};

char8_t to_hex_digit(int d)
{
  if (d < 10)
    return '0' + d;
  return 'A' + d - 10;
}

std::u8string to_hex_string(char8_t c)
{
  std::u8string hex_string;
  hex_string += to_hex_digit(c / 16);
  hex_string += to_hex_digit(c % 16);
  return hex_string;
}

Dictionary::Dictionary(std::u8string const& in)
{
  // Run over each glyph in the input.
  int glen;     // The number of bytes of the current glyph.
  for (char8_t const* glyph = in.data(); *glyph; glyph += glen)
  {
    glen = utf8_glyph_length(glyph);
    m_words.emplace_back(glyph, glen);
  }
}

void Dictionary::add(std::u8string_view glyph)
{
  if (find(glyph) == -1)
    m_words.push_back(glyph);
}

int Dictionary::find(std::u8string_view glyph) const
{
  for (int index = 0; index < m_words.size(); ++index)
    if (m_words[index] == glyph)
      return index;
  return -1;
}

} // namespace utils::detail

// Copy str to the returned filename, replacing every occurance of
// the utf8 glyphs in `from` with the corresponding one in `to`.
//
// All glyphs in `illegal` will be escaped with a percentage sign (%)
// followed by two hexidecimal characters for each code point of
// the glyph.
//
// Regardless of what is in `from`, each '%' will be replaced with "%%".
//
// All glyphs in `to` that are not in `from` are considered illegal
// and will also be escaped.
//
std::filesystem::path utf8_string_to_filename(std::u8string const& str, std::u8string const& illegal, std::u8string const& from, std::u8string const& to)
{
  using namespace detail::us2f;

  // All glyphs are found by their first byte.
  // Build a dictionary for each of the three strings.
  Dictionary from_dictionary(from);
  Dictionary to_dictionary(to);
  Dictionary illegal_dictionary(illegal);

  // The escape character is always illegal (is not allowed to appear on its own in the output).
  illegal_dictionary.add({ &escape, 1 });

  // For each `from` entry there must exist one `to` entry.
  ASSERT(from_dictionary.size() == to_dictionary.size());

  std::filesystem::path filename;

  // Run over all glyphs in the input string.
  int glen;     // The number of bytes of the current glyph.
  for (char8_t const* gp = str.data(); *gp; gp += glen)
  {
    glen = utf8_glyph_length(gp);
    std::u8string_view glyph(gp, glen);
    // Perform translation.
    int from_index = from_dictionary.find(glyph);
    if (from_index != -1)
      glyph = to_dictionary[from_index];
    else if (*gp == escape)
    {
      filename += escape;
      filename += escape;
      continue;
    }
    // What is in illegal is *always* illegal - even when it is the result of a translation.
    if (illegal_dictionary.find(glyph) != -1 ||
        // If an input glyph is not in the from_dictionary (aka, it wasn't just translated) but
        // it is in the to_dictionary - then also escape it. This is necessary to make sure that
        // each unique input str results in a unique filename (and consequently is reversable).
        (from_index == -1 && to_dictionary.find(glyph) != -1))
    {
      // Escape illegal glyphs.
      // Always escape the original input (not a possible translation), otherwise
      // we can't know if what the input was when decoding: the input could have been
      // translated first or not.
      for (int j = 0; j < glen; ++j)
      {
        filename += escape;
        filename += to_hex_string(gp[j]);
      }
      continue;
    }
    // Append the glyph to the filename.
    filename += glyph;
  }

  return filename;
}

std::u8string utf8_filename_to_string(std::filesystem::path const& filename, std::u8string const& from, std::u8string const& to)
{
  using namespace detail::us2f;

  std::u8string input = filename.u8string();
  std::u8string result;

  Dictionary from_dictionary(from);
  Dictionary to_dictionary(to);

  // First unescape all bytes in the filename.
  int glen;     // The number of bytes of the current glyph.
  for (char8_t const* gp = input.c_str(); *gp; gp += glen)
  {
    glen = utf8_glyph_length(gp);
    std::u8string_view glyph(gp, glen);
    // First translate escape sequences back - those are then always original input.
    if (*gp == escape)
    {
      if (gp[1] == escape)
      {
        glen = 2;       // Skip the second escape character too.
        result += escape;
      }
      else
      {
        char8_t val = 0;
        for (int d = 1; d <= 2; ++d)
        {
          val <<= 4;
          val |= ('0' <= gp[d] && gp[d] <= '9') ? gp[d] - '0' : gp[d] - 'A' + 10;
        }
        result += val;
        glen = 3;       // Skip the two hex digits too.
      }
      continue;
    }
    else
    {
      // Otherwise - if the character is in the from dictionary, it must have
      // been translated - otherwise it would have been escaped.
      int from_index = from_dictionary.find(glyph);
      if (from_index != -1)
        glyph = to_dictionary[from_index];
    }
    result += glyph;
  }
  return result;
}

} // namespace utils
