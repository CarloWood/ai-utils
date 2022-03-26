#pragma once

#include <utility>

// Copied from https://github.com/WG21-SG14/SG14/blob/master/SG14/algorithm_ext.h
//
// This allows for fast removal of elements from a vector by replacing them
// with elements from the end, and then remove the tail in a single erase.
//
// Usage:
//
// std::vector<int> v = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
// v.erase(utils::unstable_remove_if(v.begin(), v.end(), [](int v){ return !(v & 1); }), v.end());
//
// The order of the elements is not retained.
// After the above v would contain: { 1, 9, 3, 7, 5 } where the 2 was overwritten with 9 and 4 was overwritten with 7.
//
namespace utils {

template <class BidirIt, class UnaryPredicate>
BidirIt unstable_remove_if(BidirIt first, BidirIt last, UnaryPredicate p)
{
  while (true)
  {
    // Find the first instance of "p".
    while (true)
    {
      if (first == last) { return first; }
      if (p(*first)) { break; }
      ++first;
    }
    // Find the last instance of "not p".
    while (true)
    {
      --last;
      if (first == last) { return first; }
      if (!p(*last)) { break; }
    }
    // Move the latter over of the former.
    *first = std::move(*last);
    ++first;
  }
}

template <class BidirIt, class Val>
BidirIt unstable_remove(BidirIt first, BidirIt last, Val const& v)
{
  while (true)
  {
    // Find the first instance of "v".
    while (true)
    {
      if (first == last) { return first; }
      if (*first == v) { break; }
      ++first;
    }
    // Find the last instance of "not v".
    while (true)
    {
      --last;
      if (first == last) { return first; }
      if (!(*last == v)) { break; }
    }
    // Move the latter over the former.
    *first = std::move(*last);
    ++first;
  }
}

}  // namespace utils
