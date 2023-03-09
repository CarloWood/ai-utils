#pragma once

#include <concepts>

namespace utils {

template<typename Category, std::integral INT>
class Index
{
 private:
  INT m_index;

 public:
  constexpr explicit Index(INT index) : m_index(index) { }
  constexpr INT get_value() const { return m_index; }
  Index& operator++()    { ++m_index; return *this; }
  Index& operator--()    { --m_index; return *this; }
  Index  operator++(int) { Index prev(m_index); ++m_index; return prev; }
  Index  operator--(int) { Index prev(m_index); --m_index; return prev; }
  auto operator<=>(Index rhs) { return m_index <=> rhs.m_index; }
  bool operator==(Index const&) const = default;
};

} // namespace utils
