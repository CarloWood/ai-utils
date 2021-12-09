#pragma once

namespace utils {

// See https://stackoverflow.com/a/37931904/1487069
template<std::size_t N, typename T, typename... types>
struct get_Nth_type
{
  using type = typename get_Nth_type<N - 1, types...>::type;
};

template<typename T, typename... types>
struct get_Nth_type<0, T, types...>
{
  using type = T;
};

} // namespace utils
