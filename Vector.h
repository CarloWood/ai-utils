#pragma once

#include <vector>
#include <iostream>

namespace utils {

template <typename Category>
class VectorIndex;

template<typename Category>
std::ostream& operator<<(std::ostream& os, VectorIndex<Category> const& index);

template <typename Category>
class VectorIndex {
 private:
  std::size_t m_value;

 public:
  VectorIndex() : m_value(-1) { }
  explicit VectorIndex(std::size_t value) : m_value(value) { }
  std::size_t get_value() const { return m_value; }

  VectorIndex& operator++() { ++m_value; return *this; }
  VectorIndex operator++(int) { VectorIndex old(m_value); ++m_value; return old; }
  VectorIndex& operator--() { --m_value; return *this; }
  VectorIndex operator--(int) { VectorIndex old(m_value); --m_value; return old; }

  void clear() { m_value = 0; }
  bool is_zero() const { return m_value == 0; }
  bool undefined() const { return m_value == (std::size_t)-1; }

  bool operator==(VectorIndex const& index) const { return m_value == index.m_value; }
  bool operator!=(VectorIndex const& index) const { return m_value != index.m_value; }
  bool operator<(VectorIndex const& index) const { return m_value < index.m_value; }
  bool operator>(VectorIndex const& index) const { return m_value > index.m_value; }
  bool operator<=(VectorIndex const& index) const { return m_value <= index.m_value; }
  bool operator>=(VectorIndex const& index) const { return m_value >= index.m_value; }

  friend std::ostream& operator<<<>(std::ostream& os, VectorIndex<Category> const& index);
};

template<typename Category>
std::ostream& operator<<(std::ostream& os, VectorIndex<Category> const& index)
{
  os << '#' << index.m_value;
  return os;
}

template <typename T, typename _Index = VectorIndex<T>, typename _Alloc = std::allocator<T>>
class Vector : public std::vector<T, _Alloc> {
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

  index_type ibegin() const { return index_type((size_t)0); }
  index_type iend() const { return index_type(_Base::size()); }

 private:
  reference operator[](size_type __n) _GLIBCXX_NOEXCEPT;
  const_reference operator[](size_type __n) const _GLIBCXX_NOEXCEPT;
};

} // namespace utils
