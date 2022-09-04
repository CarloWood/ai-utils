#pragma once

#include "VectorCompare.h"
#include "derived_from_template.h"
#include "is_vector.h"

namespace utils {

// PairCompare
//
// Compare two pairs pair<First, Second> where First and/or Second may be
// a std::vector<Element> or just Element (which is then treated like we would a vector of 1 element).
//
// FirstCompare and SecondCompare must compare the Element type.
//
// First compares 'First' and if equal, returns the compare of Second.
// Where compare means returning true if the lhs is "less than" the rhs.
// Both First and Second are first compared by number of elements: if the
// number of element is unequal then we return lhs.size() < rhs.size(),
// otherwise vectors are compared element by element until the first element
// that is unequal.
//
template<typename FirstCompare, typename SecondCompare>
struct PairCompare
{
  template<typename VectorFirst, typename VectorSecond>
  requires (derived_from_template_v<VectorFirst, std::vector> && derived_from_template_v<VectorSecond, std::vector>)
  bool operator()(std::pair<VectorFirst, VectorSecond> const& lhs,
                  std::pair<VectorFirst, VectorSecond> const& rhs) const
  {
    utils::VectorCompare<FirstCompare> vector_compare_first;
    if (vector_compare_first(lhs.first, rhs.first))
      return true;
    else if (vector_compare_first(rhs.first, lhs.first))
      return false;

    utils::VectorCompare<SecondCompare> vector_compare_second;
    return vector_compare_second(lhs.second, rhs.second);
  }

  template<typename VectorFirst, typename Second>
  requires (derived_from_template_v<VectorFirst, std::vector> && !utils::is_vector_v<Second>)
  bool operator()(std::pair<VectorFirst, Second> const& lhs,
                  std::pair<VectorFirst, Second> const& rhs) const
  {
    utils::VectorCompare<FirstCompare> vector_compare_first;
    if (vector_compare_first(lhs.first, rhs.first))
      return true;
    else if (vector_compare_first(rhs.first, lhs.first))
      return false;

    return lhs.second < rhs.second;
  }

  template<typename First, typename VectorSecond>
  requires (!utils::is_vector_v<First> && utils::derived_from_template_v<VectorSecond, std::vector>)
  bool operator()(std::pair<First, VectorSecond> const& lhs,
                  std::pair<First, VectorSecond> const& rhs) const
  {
    if (lhs.first < rhs.first)
      return true;
    if (rhs.first < lhs.first)
      return false;

    utils::VectorCompare<SecondCompare> vector_compare_second;
    return vector_compare_second(lhs.second, rhs.second);
  }

  template<typename First, typename Second>
  requires (!utils::is_vector_v<First> && !utils::is_vector_v<Second>)
  bool operator()(std::pair<First, Second> const& lhs,
                  std::pair<First, Second> const& rhs) const
  {
    if (lhs.first < rhs.first)
      return true;
    if (rhs.first < lhs.first)
      return false;
    return lhs.second < rhs.second;
  }
};

} // namespace utils
