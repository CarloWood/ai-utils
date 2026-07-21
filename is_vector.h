// SPDX-FileCopyrightText: 2022 Carlo Wood
// SPDX-License-Identifier: MIT

#pragma once

#include "Vector.h"
#include <type_traits>

// Copied from https://stackoverflow.com/a/72814709/1487069
namespace utils {

template <typename C> struct is_vector : std::false_type { };
template <typename T, typename A> struct is_vector<std::vector<T, A>> : std::true_type { };
template <typename T, typename I, typename A> struct is_vector<utils::Vector<T, I, A>> : std::true_type { };
template <typename C> constexpr bool is_vector_v = is_vector<C>::value;

} // namespace utils
