#pragma once

namespace utils {

// Usage:
//
// Dout(dc::notice, utils::print_using(some_container, utils::QuotedList{.open="[ ", .separator="|"}));
//
// which will list the elements of some_container on the current ostream,
// with the 'open' string on the left, the 'close' string on the right and
// the 'separator' as separator.

struct QuotedList
{
  char const* open = "{ ";
  char const* separator = ", ";
  char const* close = " }";

  template<typename Container>
  void operator()(std::ostream& os, Container const& container)
  {
    os << open;
    char const* prefix = "";
    for (auto&& element : container)
    {
      os << prefix << '"' << element << '"';
      prefix = separator;
    }
    os << close;
  }
};

} // namespace utils
