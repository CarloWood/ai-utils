#pragma once

#include <vector>

namespace utils {

template<typename ElementCompare>
struct VectorCompare
{
  static constexpr ElementCompare element_compare{};

  template<typename T>
  bool operator()(std::vector<T> const& lhs, std::vector<T> const& rhs) const
      requires requires(T e1, T e2) { ElementCompare{}(e1, e2); }
  {
    if (lhs.size() != rhs.size())
      return lhs.size() < rhs.size();
    auto lhsi = lhs.begin();
    auto rhsi = rhs.begin();
    while (lhsi != lhs.end())
    {
      if (element_compare(*lhsi, *rhsi))
        return true;
      else if (element_compare(*rhsi, *lhsi))
        return false;
      ++lhsi;
      ++rhsi;
    }
    return false;
  }
};

} // namespace utils
