/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of is_pointer_like.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <experimental/type_traits>
#include <type_traits>

namespace utils {

template<typename T> using dereferencable_type = decltype(*std::declval<T>());
template<typename T> using arrow_type = decltype(std::declval<T>().operator->());

template<typename T> using is_dereferencable = std::experimental::is_detected<dereferencable_type, T>;
template<typename T> using has_arrow = std::experimental::is_detected<arrow_type, T>;
template<typename T> using is_pointer_like_dereferencable = is_dereferencable<T>;

template<typename T> using is_pointer_like_arrow_dereferencable = std::disjunction<std::is_pointer<T>, has_arrow<T>>;
template<typename T> using is_pointer_like = std::conjunction<is_pointer_like_dereferencable<T>, is_pointer_like_arrow_dereferencable<T>>;

template<typename T> inline constexpr bool is_pointer_like_dereferencable_v = is_pointer_like_dereferencable<T>::value;
template<typename T> inline constexpr bool is_pointer_like_arrow_dereferencable_v = is_pointer_like_arrow_dereferencable<T>::value;
template<typename T> inline constexpr bool is_pointer_like_v = is_pointer_like<T>::value;

} // namespace utils
