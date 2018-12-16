// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class VTPtr.
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

#pragma once

// Usage:
//
// If a class is not derived (from a class that has also a VTPtr):

#ifdef USAGE_EXAMPLE1
#include "utils/VTPtr.h"

class B
{
 public:
  struct VT_type
  {
    void (*_x)(B*, int);
    void (*_of)(B*, int);
    void (*_y)(B*, int);
  };

  struct VT_impl
  {
    // Write the "default" implementations of the virtual functions.
    static void x(B* self, int a) { ... }
    static void of(B* self, int a) { ... }
    static void y(B* self, int a) { ... }

    // Virtual table of B.
    static constexpr VT_type VT{
      x,
      of,
      y
    };
  };

  utils::VTPtr<B> VT_ptr;                                                            // Virtual table pointer of this instance.

  B() : VT_ptr(this) { }                                                             // Initialize VT_ptr with `this'.

 protected:
   // For each (new) function in VT_type, add a hook:
   void x(int a) { return VT_ptr->_x(this, a); }                                     // Hooks for the new virtual functions.
   void of(int a) { return VT_ptr->_of(this, a); }
   void y(int a) { return VT_ptr->_y(this, a); }
};
#endif // USAGE_EXAMPLE1

// Lets add a pure virtual function pv and a virtual function vf,
// and lets override a virtual function of, to a class D derived
// from B:
//
#ifdef USAGE_EXAMPLE2
#include "B.h"

class D : public B
{
 public:
  struct VT_type : B::VT_type
  {
    void (*_pv)(D*, int);
    int  (*_vf)(D*, int);
  };

  struct VT_impl : B::VT_impl
  {
    static void of(B* _self, int a) { D* self = static_cast<D*>(_self); ... }        // Override a virtual function of B.
    static int  vf(D* self, int a) { return ... }                                    // Add a new virtual function of D.

    // Virtual table of D.
    static constexpr VT_type VT{
      x,       // Virtual function of B that we do not override.
      of,                                                                            // Virtual function of B that we did override.
      y,       // Virtual function of B that we do not override.
      nullptr,                                                                       // Pure virtual function that we added.
      vf                                                                             // Virtual function that we added.
    };
  };

  utils::VTPtr<D, B> VT_ptr;                                                         // Virtual table pointer of this instance.

  D() : VT_ptr(this) { }                                                             // Initialize VT_ptr with `this'.

 protected:
   void pv(int a) { return VT_ptr->_pv(this, a); }                                   // Hooks for the new virtual functions.
   int  vf(int a) { return VT_ptr->_vf(this, a); }
};
#endif // USAGE_EXAMPLE2

namespace utils {

// Virtual base class of VT_impl's.
struct VT_base
{
  virtual ~VT_base() { }
  virtual std::shared_ptr<VT_base> copy() const = 0;

  template<typename T>
  std::shared_ptr<VT_base> copy_vt() const
  {
    // FIXME
    return std::make_shared<T>();
  }
};

template<typename T>
struct HasCopyMethod
{
  template<typename U, std::shared_ptr<VT_base> (U::*)() const> struct SFINAE {};
  template<typename U> static char Test(SFINAE<U, &U::copy>*);
  template<typename U> static int Test(...);
  static const bool value = sizeof(Test<T>(nullptr)) == sizeof(char);
};

template<class Self, class Base1 = void, class Base2 = void>
struct VTPtr
{
  using VT_type = typename Self::VT_type;
  using VT_impl = typename Self::VT_impl;

  VT_type const* VT_ptr = &VT_impl::VT;                 // Virtual Table pointer.

  void set(Self* self, VT_type const* vt_ptr) { VT_ptr = vt_ptr; self->Base1::VT_ptr.set(self, VT_ptr); self->Base2::VT_ptr.set(self, VT_ptr); }
  VT_type const* operator->() { return VT_ptr; }

  VTPtr(Self* self) : VT_ptr(&VT_impl::VT) { self->Base1::VT_ptr.set(self, VT_ptr); self->Base2::VT_ptr.set(self, VT_ptr); }

  // Make sure that VT_impl has the copy() function.
  // Note that this function must be public for this test to work :/
  //
  // The last two lines of every VT_impl should be:
#if -0
    // Allow copying this virtual table.
    std::shared_ptr<utils::VT_base> copy() const override { return copy_vt<VT_impl>(); }
#endif
  static_assert(HasCopyMethod<VT_impl>::value, "VT_impl needs to override copy().");
};

template<class Self, class Base1>
struct VTPtr<Self, Base1, void>
{
  using VT_type = typename Self::VT_type;
  using VT_impl = typename Self::VT_impl;

  VT_type const* VT_ptr = &VT_impl::VT;                 // Virtual Table pointer.

  void set(Self* self, VT_type const* vt_ptr) { VT_ptr = vt_ptr; self->Base1::VT_ptr.set(self, VT_ptr); }
  VT_type const* operator->() { return VT_ptr; }

  VTPtr(Self* self) : VT_ptr(&VT_impl::VT) { self->Base1::VT_ptr.set(self, VT_ptr); }

  // Make sure that VT_impl has the copy() function.
  // Note that this function must be public for this test to work :/
  //
  // The last two lines of every VT_impl should be:
#if -0
    // Allow copying this virtual table.
    std::shared_ptr<utils::VT_base> copy() const override { return copy_vt<VT_impl>(); }
#endif
  static_assert(HasCopyMethod<VT_impl>::value, "VT_impl needs to override copy().");
};

template<class Self>
struct VTPtr<Self, void, void>
{
  using VT_type = typename Self::VT_type;
  using VT_impl = typename Self::VT_impl;

  VT_type const* VT_ptr = &VT_impl::VT;                 // Virtual Table pointer.

  void set(Self*, VT_type const* vt_ptr) { VT_ptr = vt_ptr; }
  VT_type const* operator->() { return VT_ptr; }

  VTPtr(Self*) : VT_ptr(&VT_impl::VT) { }

  // Make sure that VT_impl has the copy() function.
  // Note that this function must be public for this test to work :/
  //
  // The last two lines of every VT_impl should be:
#if -0
    // Allow copying this virtual table.
    std::shared_ptr<utils::VT_base> copy() const override { return copy_vt<VT_impl>(); }
#endif
  static_assert(HasCopyMethod<VT_impl>::value, "VT_impl needs to override copy().");
};

} // namespace utils
