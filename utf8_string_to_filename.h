#pragma once

#include <filesystem>
#include <string>

namespace utils {

// Convert any utf8 string to a legal filename.
//
// The default values:
// spaces are replaced with underscores and underscores are replaced with a double underscore (\u2017).
// A slash (/) is always illegal, but is replaced with a 'division slash'. It is not really necessary
// to pass u8"/" to `illegal` therefore, but that default is needed when `from` is changed to not
// include a '/'.
//
std::filesystem::path utf8_string_to_filename(
    std::u8string const& str, std::u8string const& illegal = u8"/", std::u8string const& from = u8" _/", std::u8string const& to = u8"_\u2017\u2215");

// The reverse of the above. Decodes the translated/escaped filename back to the original string provided
// the same translation strings are used (but swapped).
std::u8string utf8_filename_to_string(std::filesystem::path const& filename, std::u8string const& from = u8"_\u2017\u2215", std::u8string const& to = u8" _/");

} // namespace utils
