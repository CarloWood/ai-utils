#pragma once

#include <concepts>
#include <type_traits>

namespace utils {

template<typename T>
concept ConceptMultiplicableAssignable = requires(T a, T b)
{
  { a *= b } -> std::convertible_to<T>;
};

template<typename T>
concept ConceptBuiltin = std::is_integral_v<T> || std::is_floating_point_v<T>;

// Returns the square of its argument.
template<typename T>
requires ConceptMultiplicableAssignable<T> && (!ConceptBuiltin<T>)
T square(T const& arg)
{
  T result(arg);
  result *= arg;
  return result;
}

template<typename T>
requires (!ConceptMultiplicableAssignable<T>) && (!ConceptBuiltin<T>)
consteval auto square(T const& arg)
{
  return arg * arg;
}

template<ConceptBuiltin T>
T square(T arg)
{
  return arg * arg;
}

} // namespace utils
