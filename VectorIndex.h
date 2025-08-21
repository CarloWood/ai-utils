/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of template class VectorIndex in namespace utils.
 *
 * @Copyright (C) 2017  Carlo Wood.
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

#include "itoa.h"
#include <iostream>

namespace utils {

template <typename Category>
class VectorIndex;

template<typename Category>
std::ostream& operator<<(std::ostream& os, VectorIndex<Category> const& index);

template <typename Category>
class VectorIndex
{
 private:
  std::size_t m_value;

 public:
  constexpr VectorIndex() : m_value(-1) { }
  explicit constexpr VectorIndex(std::size_t value) : m_value(value) { }
  constexpr std::size_t get_value() const { return m_value; }
  explicit constexpr operator std::size_t() const { return m_value; }

  VectorIndex& operator++() { ++m_value; return *this; }
  VectorIndex operator++(int) { VectorIndex old(m_value); ++m_value; return old; }
  VectorIndex& operator--() { --m_value; return *this; }
  VectorIndex operator--(int) { VectorIndex old(m_value); --m_value; return old; }

  void set_to_zero() { m_value = 0; }
  bool is_zero() const { return m_value == 0; }
  void set_to_undefined() { m_value = -1; }
  bool undefined() const { return m_value == (std::size_t)-1; }

  bool operator==(VectorIndex index) const { return m_value == index.m_value; }
  bool operator!=(VectorIndex index) const { return m_value != index.m_value; }
  bool operator<(VectorIndex index) const { return m_value < index.m_value; }
  bool operator>(VectorIndex index) const { return m_value > index.m_value; }
  bool operator<=(VectorIndex index) const { return m_value <= index.m_value; }
  bool operator>=(VectorIndex index) const { return m_value >= index.m_value; }

  constexpr VectorIndex operator<<(int n) const { return VectorIndex{m_value << n}; }
  constexpr VectorIndex operator>>(int n) const { return VectorIndex{m_value >> n}; }
  friend constexpr VectorIndex operator+(VectorIndex lhs, ssize_t n) { return VectorIndex{lhs.m_value + n}; }
  friend constexpr VectorIndex operator-(VectorIndex lhs, ssize_t n) { return VectorIndex{lhs.m_value - n}; }
  friend constexpr VectorIndex operator+(VectorIndex lhs, size_t n) { return VectorIndex{lhs.m_value + n}; }
  friend constexpr VectorIndex operator-(VectorIndex lhs, size_t n) { return VectorIndex{lhs.m_value - n}; }
  friend constexpr VectorIndex operator+(ssize_t n, VectorIndex rhs) { return VectorIndex{n + rhs.m_value}; }
  friend constexpr VectorIndex operator-(ssize_t n, VectorIndex rhs) { return VectorIndex{n - rhs.m_value}; }
  friend constexpr VectorIndex operator+(size_t n, VectorIndex rhs) { return VectorIndex{n + rhs.m_value}; }
  friend constexpr VectorIndex operator-(size_t n, VectorIndex rhs) { return VectorIndex{n - rhs.m_value}; }
  friend constexpr ssize_t operator-(VectorIndex lhs, VectorIndex rhs) { return lhs.m_value - rhs.m_value; }
  constexpr VectorIndex operator%(size_t n) const { return VectorIndex{m_value % n}; }
  constexpr VectorIndex operator%(VectorIndex index) const { return VectorIndex{m_value % index.m_value}; }
  friend constexpr VectorIndex operator*(VectorIndex lhs, size_t n) { return VectorIndex{lhs.m_value * n}; }
  friend constexpr VectorIndex operator*(size_t n, VectorIndex rhs) { return VectorIndex{n * rhs.m_value}; }
  constexpr VectorIndex operator/(size_t n) const { return VectorIndex{m_value / n}; }
  friend constexpr VectorIndex operator|(VectorIndex lhs, VectorIndex rhs) { return VectorIndex{lhs.m_value | rhs.m_value}; }
  friend constexpr VectorIndex operator&(VectorIndex lhs, VectorIndex rhs) { return VectorIndex{lhs.m_value & rhs.m_value}; }
  friend constexpr VectorIndex operator|(VectorIndex lhs, size_t n) { return VectorIndex{lhs.m_value | n}; }
  friend constexpr VectorIndex operator&(VectorIndex lhs, size_t n) { return VectorIndex{lhs.m_value & n}; }
  friend constexpr VectorIndex operator|(size_t n, VectorIndex rhs) { return VectorIndex{n | rhs.m_value}; }
  friend constexpr VectorIndex operator&(size_t n, VectorIndex rhs) { return VectorIndex{n & rhs.m_value}; }

  VectorIndex& operator<<=(int n) { m_value <<= n; return *this; }
  VectorIndex& operator>>=(int n) { m_value >>= n; return *this; }
  VectorIndex& operator+=(ssize_t n) { m_value += n; return *this; }
  VectorIndex& operator-=(ssize_t n) { m_value -= n; return *this; }
  VectorIndex& operator+=(size_t n) { m_value += n; return *this; }
  VectorIndex& operator-=(size_t n) { m_value -= n; return *this; }
  VectorIndex& operator-=(VectorIndex index) { ASSERT(m_value >= index.m_value); m_value -= index.m_value; return *this; }
  VectorIndex& operator%=(size_t n) { m_value %= n; return *this; }
  VectorIndex& operator%=(VectorIndex index) { m_value %= index.m_value; return *this; }
  VectorIndex& operator*=(size_t n) { m_value *= n; return *this; }
  VectorIndex& operator/=(size_t n) { m_value /= n; return *this; }
  VectorIndex& operator|=(VectorIndex index) { m_value |= index.m_value; return *this; }
  VectorIndex& operator&=(VectorIndex index) { m_value &= index.m_value; return *this; }
  VectorIndex& operator|=(size_t n) { m_value |= n; return *this; }
  VectorIndex& operator&=(size_t n) { m_value &= n; return *this; }

  friend std::ostream& operator<<<>(std::ostream& os, VectorIndex<Category> const& index);
};

template<typename Category>
std::ostream& operator<<(std::ostream& os, VectorIndex<Category> const& index)
{
  if (index.undefined())
    os << "<undefined>";
  else
    os << '#' << index.m_value;
  return os;
}

template<typename Category>
std::string to_string(VectorIndex<Category> const& index)
{
  std::string result("#");
  if (index.undefined())
    result = "<undefined>";
  else
  {
    std::array<char, 21> buf;                   // 20 = number of chars in decimal of size_t(-1). Plus one for terminating zero.
    result += itoa(buf, index.get_value());
  }
  return result;
}

} // namespace utils
