// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of template classes HasDerivedFrom and DerivedFrom in namespace utils.
//
// Copyright (C) 2018 Carlo Wood.
//
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file is part of ai-utils.
//
// ai-utils is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ai-utils is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.

#include "debug.h"
#include "AIRefCount.h"
#include "nearest_multiple_of_power_of_two.h"
#include "apply_function.h"
#include <iostream>
#include <utility>
#include <tuple>
#include <type_traits>

// Usage:
//
// class C final : utils::HasDerivedFrom<C, B>
// {
//   ...
//
//  public:
//   C() : utils::HasDerivedFrom<C, B>() { }
//
//  private:
//   utils::DerivedFrom<B> m_member;    // Extends to beyond C!
// };
//
// And allocate on the heap with,
//
// boost::intrusive_ptr<C> a = C::create<T>(1, 2)(3, 4);
//
// Where (1, 2) are the constructor arguments of C and (3, 4) the
// constructor arguments of type T, which must be derived from B.
//
// After this m_member can be viewed as
//
// B* b = new T(3, 4);
// B& m_member(*b);
//
// Except that as far the compiler is concerned it
// is a true member of C (not a reference or pointer).
// However, sizeof(C) will fail (that sees m_member as
// having a size of 1).
//

namespace utils {

namespace {

template<std::size_t alignment, std::size_t size, typename C>
struct alignas(alignment) Padding : public C
{
  static_assert(alignment >= alignof(C), "");
  static_assert(size > sizeof(C), "");
  char m_padding[size - sizeof(C)];

  using C::C;
};

} // namespace

//-----------------------------------------------------------------------------
// struct DerivedFrom<B>
//
// A member variable of an unspecified type, except that it is derived from B.
// The actual type could be larger than B. It is possible to specify this extra
// space, but that is not necessary as long as this is the last variable in
// a class C (although, you can't use sizeof(C) reliably in that case).
//
// The containing class (class C) will be dynamically allocated.

template<typename B>
struct alignas(B) DerivedFrom final
{
  operator B&() { return reinterpret_cast<B&>(*this); }
  operator B const&() const { return reinterpret_cast<B const&>(*this); }
};

//-----------------------------------------------------------------------------
// class HasDerivedFrom<C, B>
//
// Base class for class C whose last member has the type DerivedFrom<B>.

template<typename C, typename B>                // C is the final class that Contains the DerivedFrom<B> member.
class HasDerivedFrom : public AIRefCount
{
 protected:
  B* m_derived_from_ptr;

  template<typename T, typename... Args_C>      // T is the type derived from B; Args_C are the parameters of the constructor of C.
  struct create_return_type
  {
    std::tuple<Args_C...> m_args_C;

    create_return_type(Args_C&&... args_C) : m_args_C(std::make_tuple(std::forward<Args_C>(args_C)...)) { }

    template<std::size_t sizeof_C2, std::size_t alignof_C2, std::size_t... Is>
    C* construct_at(std::tuple<Args_C...>& args_C, index_sequence<Is...> const) // Special case of utils::dispatch_function.
    {
      return new Padding<alignof_C2, sizeof_C2, C>(std::get<Is>(args_C)...);
    }

    template<typename... Args_T>
    C* operator()(Args_T... args_T);
  };

 public:
  HasDerivedFrom(DerivedFrom<B>& df) : m_derived_from_ptr(reinterpret_cast<B*>(&df)) { }

  template<typename T, typename... Args_C>      // T is the type derived from B; Args_C are the parameters of the constructor of C.
  static create_return_type<T, Args_C...> create(Args_C&&... args_C)
  {
    return create_return_type<T, Args_C...>(std::forward<Args_C>(args_C)...);
  }
};

template<typename C, typename B>
template<typename T, typename... Args_C>
template<typename... Args_T>
C* HasDerivedFrom<C, B>::create_return_type<T, Args_C...>::operator()(Args_T... args_T)
{
  // We know that C is defined as,
  //
  // class C : public HasDerivedFrom<C, B> {
  //   /* Arbitrary stuff */
  //   DerivedFrom<B> m_...;    // This is the last member of the class.
  // };
  //
  // And we need to mimick that this is,
  //
  // class C2 : public HasDerivedFrom<C, B> {
  //   /* The same arbitrary stuff */
  //   T m_...;
  // };
  //
  // The alignment of C2 is trivial:
  constexpr size_t alignof_C2 = std::max(alignof(C), alignof(T));
  // The offsetof(C, m_...) can also be calculated (using the fact that alignof() is a power of two):
  constexpr size_t offsetof_C_m = nearest_multiple_of_power_of_two(sizeof(C) - sizeof(DerivedFrom<B>), alignof(DerivedFrom<B>));
  // While the required offset of m_... relative to C2 is,
  constexpr size_t offsetof_C2_m = nearest_multiple_of_power_of_two(sizeof(C) - sizeof(DerivedFrom<B>), alignof(T));
  // If the alignment of T is more strict than the alignment of DerivedFrom<B> (which is set to be alignof(B)), then
  // we might need to put C in a memory location with a certain offset (relative to the start that is aligned as alignof_C2),
  // in order to fix the alignment of m_.... This offset will be:
  constexpr size_t delta = offsetof_C2_m - offsetof_C_m;
  // The size of C2 is rather straight forward:
  constexpr size_t sizeof_C2 = offsetof_C2_m + sizeof(T);

  // If delta > 0, then it is not possible to derive a class X from C such that X is aligned at
  // alignof_C2 and C::DerivedFrom<B> will be aligned at alignof_C2 (alignof(T)) because that
  // would mean that there needs to be an offset between the start of X and the start of its base
  // class C of delta. That might happen using multiple inheritance but cannot be enforced because
  // C++ does not guarantee anything about the order in which base classes are put in memory.
  // See https://stackoverflow.com/questions/53687080/is-it-possible-with-c-to-orchestrate-the-offset-of-a-base-class-relative-to-it
  static_assert(delta == 0, "The alignment of T can not be more strict than the alignment of B.");

  C* c = construct_at<sizeof_C2, alignof_C2>(m_args_C, typename index_sequence_generator<sizeof...(Args_C)>::type{});
  ASSERT(reinterpret_cast<std::uintptr_t>(c) % alignof(C) == 0);
  AllocTag(c, "A class containing a member of type " << libcwd::type_info_of<T>().demangled_name());
  // Make sure that the alignment of the DerivedFrom<B> member is correct.
  ASSERT(reinterpret_cast<std::uintptr_t>(c->m_derived_from_ptr) % alignof(T) == 0);
  new (c->m_derived_from_ptr) T(args_T...);
  return c;
}

} // namespace utils
