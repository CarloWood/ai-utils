#pragma once

#ifdef HAVE_UTILS_CONFIG_H
#include "utils/config.h"       // Needed for HAVE_BOOST_FILESYSTEM.
#ifdef HAVE_BOOST_FILESYSTEM
#include <string>
#endif
#endif

namespace utils {

// Add `find_package(Boost COMPONENTS filesystem)` to the CMakeLists.txt file in the root of the project!
#ifdef HAVE_BOOST_FILESYSTEM
std::string generate_unique_filename(std::string const& filename);
#endif

} // namespace utils
