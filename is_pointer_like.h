// SPDX-FileCopyrightText: 2019, 2022, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of is_pointer_like.
 */

#pragma once

#include <experimental/type_traits>
#include <type_traits>

namespace utils {

template<typename T> using dereferencable_type = decltype(*std::declval<T>());
template<typename T> using operator_arrow_type = decltype(std::declval<T>().operator->());

template<typename T> using is_dereferencable = std::experimental::is_detected<dereferencable_type, T>;
template<typename T> using has_arrow = std::experimental::is_detected<operator_arrow_type, T>;
template<typename T> using is_pointer_like_dereferencable = is_dereferencable<T>;

template<typename T> using is_pointer_like_arrow_dereferencable = std::disjunction<std::is_pointer<T>, has_arrow<T>>;
template<typename T> using is_pointer_like = std::conjunction<is_pointer_like_dereferencable<T>, is_pointer_like_arrow_dereferencable<T>>;

template<typename T> inline constexpr bool is_pointer_like_dereferencable_v = is_pointer_like_dereferencable<T>::value;
template<typename T> inline constexpr bool is_pointer_like_arrow_dereferencable_v = is_pointer_like_arrow_dereferencable<T>::value;
template<typename T> inline constexpr bool is_pointer_like_v = is_pointer_like<T>::value;

} // namespace utils
