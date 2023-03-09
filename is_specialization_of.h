#pragma once

#include <type_traits>

// Usage example:
//
// template<typename T>
// inline constexpr bool is_map = utils::is_specialization_of_v<T, std::map>;
//
// template<typename T>
// concept ConceptIsMap = is_map<T>;
//
namespace utils {

template<class, template<class...> class>
inline constexpr bool is_specialization_of = false;

template<template<class...> class T, class... Args>
inline constexpr bool is_specialization_of<T<Args...>, T> = true;

template<typename T, template<typename...> typename U>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, U>;

} // namespace utils
