#pragma once

#include <array>
#include <utility>

namespace utils {
namespace detail {

template<typename T, std::size_t M, int MS, std::size_t N, std::size_t... MIndexes, std::size_t... NIndexes>
constexpr auto concat_array_impl(std::array<T, M> const& m, std::array<T, N> const& n,
    std::index_sequence<MIndexes...>, std::index_sequence<NIndexes...>)
{
  return std::array<T, M - MS + N>{m[MIndexes]..., n[NIndexes]...};
}

} // namespace detail

// Concatenate two constexpr arrays of T.
//
// Usage example:
//
//   constexpr std::array<int, 3> a1 = { 1, 2, 3 };
//   constexpr std::array<int, 4> a2 = { 4, 5, 6, 7 };
//   constexpr auto a12 = utils::concat_array(a1, a2);
//
template<typename T, std::size_t M, std::size_t N>
constexpr auto concat_array(std::array<T, M> const& m, std::array<T, N> const& n)
{
  return detail::concat_array_impl<T, M, 0, N>(m, n, std::make_index_sequence<M>(), std::make_index_sequence<N>());
}

// Concatenate two zero terminated constexpr arrays of T.
//
// The result is again zero terminated; that is, this just catenates
// the two arrays leaving out the last character of the first array.
//
// Usage example:
//
//   constexpr auto s1 = std::to_array("Hello ");
//   constexpr auto s2 = std::to_array("world");
//   constexpr auto s12 = utils::zconcat_array(s1, s2);
//   constexpr char const* hw = s12.data();
//
template<typename T, std::size_t M, std::size_t N>
constexpr auto zconcat_array(std::array<T, M> const& m, std::array<T, N> const& n)
{
  return detail::concat_array_impl<T, M, 1, N>(m, n, std::make_index_sequence<M - 1>(), std::make_index_sequence<N>());
}

} // namespace utils
