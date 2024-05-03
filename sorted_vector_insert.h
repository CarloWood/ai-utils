#pragma once

#include <vector>
#include <algorithm>

// Copied from https://stackoverflow.com/a/25524075/1487069
namespace utils {

template<typename T>
typename std::vector<T>::iterator sorted_vector_insert(std::vector<T>& vec, T const& item)
{
  return vec.insert(std::upper_bound(vec.begin(), vec.end(), item), item);
}

template<typename T, typename Compare>
typename std::vector<T>::iterator sorted_vector_insert(std::vector<T>& vec, T const& item, Compare compare)
{
  return vec.insert(std::upper_bound(vec.begin(), vec.end(), item, compare), item);
}

} // namespace utils
