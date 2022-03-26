#include "sys.h"
#include "macros.h"

namespace utils {

// Returns the length of the UTF8 encoded glyph, which is highly
// recommended to be either guaranteed correct UTF8, or points
// inside a zero terminated string.
//
// If the pointer does not point to a legal UTF8 glyph then 1 is returned.
// The zero termination is necessary to detect the end of the string
// in the case that the apparent encoded glyph length goes beyond the string.
//
int utf8_glyph_length(char8_t const* glyph)
{
  // The length of a glyph is determined by the first byte.
  // This magic formula returns 1 for 110xxxxx, 2 for 1110xxxx, 3 for 11110xxx and 0 otherwise.
  int extra = (0x3a55000000000000 >> ((*glyph >> 2) & 0x3e)) & 0x3;
  // Detect if there are indeed `extra` bytes that follow the first one, each of which must begin with 10xxxxxx to be legal UTF8.
  int i = 0;
  while (++i <= extra)
    if (AI_UNLIKELY(glyph[i] >> 6 != 2))
      return 1;         // Not legal UTF8 encoding.
  return 1 + extra;
}

} // namespace utils
