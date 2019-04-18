// Copied from https://github.com/PeterSommerlad/ReverseAdapter
// License: MIT

// Changes by Carlo Wood,
// * renamed the namespace to adaptor as that is the word used by the standard (as opposed to adapter).
// * white space formatting
// * Added comments (partially also from Peter's repository).
//
// Usage:
//
// using ::adaptor::reversed;
// for (auto const& i : reversed({0,1,2,3,4,5}))
//   // use 'i'

// As of C++20 this header might become obsolete because then there
// will be a std::reverse range adaptor that will provide almost the
// same, except that you must ensure lifetimes yourself. For example,
// by using the initializer in the range for.

#pragma once

#include <type_traits>
#include <initializer_list>
#include <utility>
#include <iterator>

namespace adaptor {

template<typename Cont>
struct reversed
{
  explicit constexpr reversed(Cont& c) : container(c) { }
  explicit constexpr reversed(std::remove_reference_t<Cont>&& c) : container(std::move(c)) { }

  Cont container;
  constexpr auto begin() { return std::rbegin(container); }
  constexpr auto begin() const { return std::rbegin(container); }
  constexpr auto end() { return std::rend(container); }
  constexpr auto end() const { return std::rend(container); }
};

// The C++17 deduction guides ensure that temporaries will be kept
// around in the reverse object and not dangle. Non-temporaries will
// be kept by reference without overhead:

template<typename Cont>
reversed(Cont&) -> reversed<Cont&>;

template<typename Cont>
reversed(Cont&&) -> reversed<Cont>;

template<typename T>
reversed(std::initializer_list<T>) -> reversed<std::initializer_list<T>>;

} // namespace adaptor
