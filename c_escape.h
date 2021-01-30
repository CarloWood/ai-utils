#pragma once

#include "c_escape_iterator.h"
#include <string_view>
#include <algorithm>
#include <iostream>

namespace utils {

template<typename T>
void c_escape(std::ostream& os, T const& data)
{
  using it_escaped_t = c_escape_iterator<typename T::const_iterator>;
  std::copy(it_escaped_t(data.begin(), data.end()), it_escaped_t(data.end()), std::ostreambuf_iterator<char>(os));
}

} // namespace utils
