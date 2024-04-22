#include "sys.h"
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

namespace utils {

namespace fs = boost::filesystem;

// Returns the same filename as passed, unless it already exists;
// in that case a number between parenthesis is added to make it unique
// (not yet existing).
std::string generate_unique_filename(std::string const& filename)
{
  fs::path filePath(filename);

  // If file doesn't exist, return the original filename.
  if (!fs::exists(filePath))
    return filename;

  std::string baseName  = filePath.stem().string();
  std::string extension = filePath.extension().string();
  fs::path dirPath      = filePath.parent_path();

  // Regular expression pattern to match the base name with count number.
  boost::regex pattern("(.*)\\s\\(([0-9]+)\\)$");

  // Extract the base name without the existing count number.
  boost::smatch match;
  if (boost::regex_match(baseName, match, pattern))
    baseName = match[1];

  // Generate unique filename by appending a number before the extension.
  int count = 1;
  while (true)
  {
    std::string newName = baseName + " (" + std::to_string(count) + ")" + extension;
    fs::path newPath    = dirPath / newName;

    // Check if the new filename exists.
    if (!fs::exists(newPath))
      return newPath.string();
    ++count;
  }
}

} // namespace utils
