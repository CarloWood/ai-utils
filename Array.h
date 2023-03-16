/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of template classes ArrayIndex and Array in namespace utils.
 *
 * @Copyright (C) 2018  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
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

#include <array>
#include <iostream>
#include "debug.h"

namespace utils {

template <typename Category>
class ArrayIndex;

template<typename Category>
std::ostream& operator<<(std::ostream& os, ArrayIndex<Category> const& index);

template <typename Category>
class ArrayIndex
{
 private:
  int m_value;

 public:
  constexpr ArrayIndex() : m_value(-1) { }
  explicit constexpr ArrayIndex(int value) : m_value(value) { }
  constexpr int get_value() const { return m_value; }
  explicit constexpr operator std::size_t() const { return m_value; }

  ArrayIndex& operator++() { ++m_value; return *this; }
  ArrayIndex operator++(int) { ArrayIndex old(m_value); ++m_value; return old; }
  ArrayIndex& operator--() { --m_value; return *this; }
  ArrayIndex operator--(int) { ArrayIndex old(m_value); --m_value; return old; }

  void set_to_zero() { m_value = 0; }
  bool is_zero() const { return m_value == 0; }
  void set_to_undefined() { m_value = -1; }
  bool undefined() const { return m_value == -1; }

  constexpr bool operator==(ArrayIndex const& index) const { return m_value == index.m_value; }
  constexpr bool operator!=(ArrayIndex const& index) const { return m_value != index.m_value; }
  constexpr bool operator<(ArrayIndex const& index) const { return m_value < index.m_value; }
  constexpr bool operator>(ArrayIndex const& index) const { return m_value > index.m_value; }
  constexpr bool operator<=(ArrayIndex const& index) const { return m_value <= index.m_value; }
  constexpr bool operator>=(ArrayIndex const& index) const { return m_value >= index.m_value; }

  ArrayIndex operator-(int n) const { return ArrayIndex{m_value - n}; }
  ArrayIndex operator+(int n) const { return ArrayIndex{m_value + n}; }

  friend std::ostream& operator<<<>(std::ostream& os, ArrayIndex<Category> const& index);
};

template<typename Category>
std::ostream& operator<<(std::ostream& os, ArrayIndex<Category> const& index)
{
  os << '#' << index.m_value;
  return os;
}

template<typename T, std::size_t N, typename _Index = ArrayIndex<T>>
class Array : public std::array<T, N>
{
 protected:
  using _Base = std::array<T, N>;

  template <std::size_t... Is>
  constexpr Array(std::initializer_list<T> ilist, std::index_sequence<Is...>) : std::array<T, N>{T{*(ilist.begin() + Is)}...}
  {
  }

 public:
  using reference = typename _Base::reference;
  using const_reference = typename _Base::const_reference;
  using index_type = _Index;

  Array() = default;
  constexpr Array(std::initializer_list<T> ilist) : Array(ilist, std::make_index_sequence<N>{})
  {
    // The number of arguments must be equal the size of the array, because
    // each is accessed in the above constructor. Unfortunately we can't pass
    // std::make_index_sequence<ilist.size()>{}.
    ASSERT(ilist.size() == N);
  }

  reference operator[](index_type __n) _GLIBCXX_NOEXCEPT { return _Base::operator[](static_cast<size_t>(__n)); }
  const_reference operator[](index_type __n) const _GLIBCXX_NOEXCEPT { return _Base::operator[](static_cast<size_t>(__n)); }

  reference at(index_type __n) { return _Base::at(static_cast<size_t>(__n)); }
  const_reference at(index_type __n) const { return _Base::at(static_cast<size_t>(__n)); }

  index_type ibegin() const { return index_type(0); }
  index_type iend() const { return index_type((int)N); }
};

} // namespace utils
