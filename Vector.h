/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of template classes VectorIndex and Vector in namespace utils.
 *
 * @Copyright (C) 2017  Carlo Wood.
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

#include "itoa.h"
#include <vector>
#include <iostream>
#include "debug.h"

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
  std::size_t get_value() const { return m_value; }

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

  VectorIndex operator-(int n) const { return VectorIndex{m_value - n}; }
  VectorIndex operator+(int n) const { return VectorIndex{m_value + n}; }
  VectorIndex operator%(VectorIndex m) const { return VectorIndex{m_value % m.m_value}; }

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

template <typename T, typename _Index = VectorIndex<T>, typename _Alloc = std::allocator<T>>
class Vector : public std::vector<T, _Alloc>
{
 protected:
  using _Base = std::vector<T, _Alloc>;
  typedef __gnu_cxx::__alloc_traits<_Alloc> _Alloc_traits;      // FIXME?

 public:
  using value_type = typename _Base::value_type;
  using pointer = typename _Base::pointer;
  using const_pointer = typename _Base::const_pointer;
  using reference = typename _Base::reference;
  using const_reference = typename _Base::const_reference;
  using iterator = typename _Base::iterator;
  using const_iterator = typename _Base::const_iterator;
  using const_reverse_iterator = typename _Base::const_reverse_iterator;
  using reverse_iterator = typename _Base::reverse_iterator;
  using size_type = typename _Base::size_type;
  using difference_type = typename _Base::difference_type;
  using allocator_type = typename _Base::allocator_type;
  using index_type = _Index;

  // Move constructor from underlaying std::vector.
  explicit Vector(std::vector<T, _Alloc>&& x) : std::vector<T, _Alloc>(std::move(x)) { }
  // Move assignment from underlaying std::vector.
  Vector& operator=(std::vector<T, _Alloc>&& __x) noexcept(_Alloc_traits::_S_nothrow_move()) { return static_cast<Vector&>(_Base::operator=(std::move(__x))); }

  Vector()
#if __cplusplus >= 201103L
      noexcept(std::is_nothrow_default_constructible<_Alloc>::value)
#endif
      : _Base() { }

  explicit Vector(allocator_type const& __a) _GLIBCXX_NOEXCEPT : _Base(__a) { }
#if __cplusplus >= 201103L
  explicit Vector(size_type __n, allocator_type const& __a = allocator_type()) : _Base(__n, __a) { }
  Vector(size_type __n, value_type const& __value, allocator_type const& __a = allocator_type()) : _Base(__n, __value, __a) { }
#else
  explicit Vector(size_type __n, value_type const& __value = value_type(), allocator_type const& __a = allocator_type()) : _Base(__n, __value, __a) { }
#endif
  Vector(Vector const& __x) : _Base(__x) { }
#if __cplusplus >= 201103L
  Vector(Vector&& __x) noexcept : _Base(std::move(__x)) { }
  Vector(Vector const& __x, allocator_type const& __a) : _Base(__x.size(), __a) { }
  Vector(Vector&& __rv, allocator_type const& __m) noexcept(_Alloc_traits::_S_always_equal()) : _Base(std::move(__rv), __m) { }
  Vector(std::initializer_list<value_type> __l, allocator_type const& __a = allocator_type()) : _Base(__l, __a) { }
#endif
#if __cplusplus >= 201103L
  template <typename _InputIterator, typename = std::_RequireInputIter<_InputIterator>>
  Vector(_InputIterator __first, _InputIterator __last, allocator_type const& __a = allocator_type()) : _Base(__first, __last, __a) { }
#else
  template <typename _InputIterator>
  Vector(_InputIterator __first, _InputIterator __last, allocator_type const& __a = allocator_type()) : _Base(__first, __last, __a) { }
#endif

  Vector& operator=(Vector const& __x) { return static_cast<Vector&>(_Base::operator=(__x)); }
#if __cplusplus >= 201103L
  Vector& operator=(Vector&& __x) noexcept(_Alloc_traits::_S_nothrow_move()) { return static_cast<Vector&>(_Base::operator=(std::move(__x))); }
  Vector& operator=(std::initializer_list<value_type> __l) { return static_cast<Vector&>(_Base::operator=(__l)); }
#endif

 public:
  reference operator[](index_type __n) _GLIBCXX_NOEXCEPT { return _Base::operator[](__n.get_value()); }
  const_reference operator[](index_type __n) const _GLIBCXX_NOEXCEPT { return _Base::operator[](__n.get_value()); }

  reference at(index_type __n) { return _Base::at(__n.get_value()); }
  const_reference at(index_type __n) const { return _Base::at(__n.get_value()); }

  index_type ibegin() const { return index_type((size_t)0); }
  index_type iend() const { return index_type(_Base::size()); }
};

} // namespace utils
