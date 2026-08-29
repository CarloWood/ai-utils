// SPDX-FileCopyrightText: 2017-2019, 2021-2022, 2025-2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of template class Vector in namespace utils.
 */

#pragma once

#include "VectorIndex.h"
#include <vector>

namespace utils {

// This vector only accepts a certain type as index for Vector::operator[] and Vector::at, as opposed to size_t.
//
// Usage
// =====
//
// Declaration
// -----------
//
// For a certain category, which can be any type (even an incomplete one), define an index type:
//
//   struct FooBarCategory { };
//   using FooBarIndex = utils::VectorIndex<FooBarCategory>;
//
// Then define one or more Vector types that will accept this index type; e.g.
//
//   using foos_type = utils::Vector<Foo, FooBarIndex>;
//   using bars_type = utils::Vector<Bar, FooBarIndex>;
//
//   foos_type foos;
//   bars_type bars;
//
// If no index type is specified then category used is utils::VectorIndex<T>, where T is the first
// template parameter of the utils::Vector<T> (aka, the type of the elements stored in the Vector).
// Hence, there is only really a need for a separate index type if you want to use the same index
// for more than one Vector type.
//
// Either way, the index type is available from the vector type as utils::Vector<T>::index_type.
//
// Looping
// -------
//
// The run over elements of the vector by index, you can do
//
//   for (auto i = foos.ibegin(); i != foos.iend(); ++i)
//     // Use foos[i];

template <typename T, typename _Index = VectorIndex<T>, typename _Alloc = std::allocator<T>>
class Vector : public std::vector<T, _Alloc>
{
 protected:
  using _Base = std::vector<T, _Alloc>;

 public:
  using reference = typename _Base::reference;
  using const_reference = typename _Base::const_reference;
  using index_type = _Index;

  using std::vector<T, _Alloc>::vector;
  using std::vector<T, _Alloc>::operator=;

 public:
  reference operator[](index_type __n) _GLIBCXX_NOEXCEPT { return _Base::operator[](static_cast<size_t>(__n)); }
  const_reference operator[](index_type __n) const _GLIBCXX_NOEXCEPT { return _Base::operator[](static_cast<size_t>(__n)); }

  reference at(index_type __n) { return _Base::at(static_cast<size_t>(__n)); }
  const_reference at(index_type __n) const { return _Base::at(static_cast<size_t>(__n)); }

  index_type ibegin() const { return index_type(size_t{0}); }
  index_type iend() const { return index_type(_Base::size()); }
};

} // namespace utils
