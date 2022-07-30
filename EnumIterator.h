#pragma once

namespace utils {

// This assumes a contiguous range of enum values, each one larger than the previous value.
//
// Usage:
//
// enum Foo
// {
//   first = 42,
//   second,
//   last,
//   beyond_last
// };
//
// int main()
// {
//   for (Foo foo : utils::EnumIterator<Foo, Foo::first, Foo::last>())
//   {
//     std::cout << "foo = " << (int)foo << '\n';
//   }
// }
//
template<typename T, T Begin, T Last>
class EnumIterator
{
  using val_t = typename std::underlying_type<T>::type;
  val_t m_val;

 public:
  constexpr EnumIterator() : m_val(static_cast<val_t>(Begin)) { }
  constexpr EnumIterator(T const begin, int increment = 0) : m_val(static_cast<val_t>(begin) + increment) { }

  static constexpr EnumIterator s_end{Last, 1};

  EnumIterator& operator++()
  {
    ++m_val;
    return *this;
  }

  EnumIterator operator++(int)
  {
    EnumIterator prev(*this);
    ++m_val;
    return prev;
  }

  EnumIterator& operator--()
  {
    --m_val;
    return *this;
  }

  EnumIterator operator--(int)
  {
    EnumIterator prev(*this);
    --m_val;
    return prev;
  }

  EnumIterator& operator+=(int increment)
  {
    m_val += increment;
    return *this;
  }

  EnumIterator& operator-=(int decrement)
  {
    m_val -= decrement;
    return *this;
  }

  EnumIterator operator+(int increment)
  {
    EnumIterator result(*this);
    result += increment;
    return result;
  }

  EnumIterator operator-(int decrement)
  {
    EnumIterator result(*this);
    result -= decrement;
    return result;
  }

  T operator*() { return static_cast<T>(m_val); }

  EnumIterator begin() const { return {}; }
  EnumIterator end() const { return s_end; }

  bool operator!=(EnumIterator iter) const { return m_val != iter.m_val; }
};

} // namespace utils
