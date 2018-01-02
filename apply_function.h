// ai-utils -- C++ Core utilities
/*
 * These utilities are available in C++17 and later.
 * Added here in order not to demand C++17 (but instead only C++11, still).
 *
 * This was pasted to me on IRC (original author moonchild) and apparently
 * falls under Public Domain (CC0) license.
 */

#pragma once

#include <cstddef>
#include <type_traits>
#include <tuple>

namespace utils
{

// Index_sequence is useful for variadic expansion of tuple indices.
template <std::size_t... SEQ> struct index_sequence { static constexpr std::size_t size() { return sizeof...(SEQ); } };

// Index_sequence_generator creates an index_sequence and maps it to Index_sequence_generator<N>::type.
template <std::size_t N, std::size_t ...SEQ> struct index_sequence_generator : index_sequence_generator<N - 1, N - 1, SEQ...> { };

// Index_sequence_generator specialization for terminal recursive case.
template <std::size_t ...SEQ> struct index_sequence_generator<0, SEQ...> { typedef index_sequence<SEQ...> type; };

// Helper for apply_function.
template <typename Func, typename ...Args, std::size_t ...IDX>
auto dispatch_function(Func f, std::tuple<Args...>& args, index_sequence<IDX...> const) -> typename std::result_of<Func(Args&...)>::type
{
  return f(std::get<IDX>(args)...);
}

// Apply_function calls a function with the arguments supplied as a tuple.
template <typename Func, typename ...Args>
auto apply_function(Func f, std::tuple<Args...>& args) -> typename std::result_of<Func(Args&...)>::type
{
  typename index_sequence_generator<sizeof...(Args)>::type indexes;
  return dispatch_function(f, args, indexes);
}

} // namespace utils
