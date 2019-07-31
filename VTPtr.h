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

    #define VT_B { x, of, y }
  };

  struct VT_impl
  {
    // Write the "default" implementations of the virtual functions.
    static void x(B* self, int a) { ... }
    static void of(B* self, int a) { ... }
    static void y(B* self, int a) { ... }

    // Virtual table of B.
    static constexpr VT_type VT VT_B;
  };

  // These two lines must be in THIS order (clone_VT first)!
  virtual VT_type* clone_VT() { return VT_ptr.clone(this); }                         // Make a deep copy of VT_ptr.
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

    #define VT_D { VT_B, pv, vf }
  };

  struct VT_impl : B::VT_impl
  {
    static void of(B* _self, int a) { D* self = static_cast<D*>(_self); ... }        // Override a virtual function of B.
    static int  vf(D* self, int a) { return ... }                                    // Add a new virtual function of D.

    // Pure virtual function.
    static constexpr std::nullptr_t vf = nullptr;

    // Virtual table of D.
    static constexpr VT_type VT VT_D;
  };

  // These two lines must be in THIS order (clone_VT first)!
  VT_type* clone_VT() override { return VT_ptr.clone(this); }                        // Make a deep copy of VT_ptr.
  utils::VTPtr<D, B> VT_ptr;                                                         // Virtual table pointer of this instance.

  D() : VT_ptr(this) { }                                                             // Initialize VT_ptr with `this'.

 protected:
  void pv(int a) { return VT_ptr->_pv(this, a); }                                    // Hooks for the new virtual functions.
  int  vf(int a) { return VT_ptr->_vf(this, a); }
};
#endif // USAGE_EXAMPLE2

namespace utils {

template<typename T>
struct HasCloneMethod
{
  template<typename U, typename U::VT_type* (U::*)()> struct SFINAE {};
  template<typename U> static char Test(SFINAE<U, &U::clone_VT>*);
  template<typename U> static int Test(...);
  static const bool value = sizeof(Test<T>(nullptr)) == sizeof(char);
};

template<class Self>
struct VTPtrBase
{
  using VT_type = typename Self::VT_type;
  using VT_impl = typename Self::VT_impl;

  VT_type const* VT_ptr = &VT_impl::VT;                 // Virtual Table pointer.
  VT_type const* operator->() { return VT_ptr; }

  VTPtrBase(VT_type const* vt_ptr) : VT_ptr(vt_ptr) { }

  // If the static_assert fails, add the following two lines to class Self, *ABOVE* its declaration of VT_ptr.
#if -0
  VT_type* clone_VT() override { return VT_ptr.clone(this); }   // <-- Add this line, before:
  utils::VTPtr<Self, ...> VT_ptr;       // <-- you already have this line. Make sure 'Self' is the current class, followed by the class(es) it is derived from.
#endif
  static_assert(HasCloneMethod<Self>::value, "Class Self which has a member VTPtr<Self, ...> VT_ptr, must override void clone_VT() (*before* the declaration of VT_ptr!)");
};

template<class Self, class Base1 = void, class Base2 = void>
struct VTPtr : VTPtrBase<Self>
{
  void set(Self* self, typename Self::VT_type const* vt_ptr) { this->VT_ptr = vt_ptr; self->Base1::VT_ptr.set(self, this->VT_ptr); self->Base2::VT_ptr.set(self, this->VT_ptr); }
  VTPtr(Self* self) : VTPtrBase<Self>(&Self::VT_impl::VT) { self->Base1::VT_ptr.set(self, this->VT_ptr); self->Base2::VT_ptr.set(self, this->VT_ptr); }

  typename Self::VT_type* clone(Self* self)
  {
    typename Self::VT_type* new_vt_ptr = new typename Self::VT_type(*this->VT_ptr);
    this->VT_ptr = new_vt_ptr;
    self->Base1::VT_ptr.set(self, this->VT_ptr);
    self->Base2::VT_ptr.set(self, this->VT_ptr);
    return new_vt_ptr;
  }
};

template<class Self, class Base1>
struct VTPtr<Self, Base1, void> : VTPtrBase<Self>
{
  void set(Self* self, typename Self::VT_type const* vt_ptr) { this->VT_ptr = vt_ptr; self->Base1::VT_ptr.set(self, this->VT_ptr); }
  VTPtr(Self* self) : VTPtrBase<Self>(&Self::VT_impl::VT) { self->Base1::VT_ptr.set(self, this->VT_ptr); }

  typename Self::VT_type* clone(Self* self)
  {
    typename Self::VT_type* new_vt_ptr = new typename Self::VT_type(*this->VT_ptr);
    this->VT_ptr = new_vt_ptr;
    self->Base1::VT_ptr.set(self, this->VT_ptr);
    return new_vt_ptr;
  }
};

template<class Self>
struct VTPtr<Self, void, void> : VTPtrBase<Self>
{
  void set(Self*, typename Self::VT_type const* vt_ptr) { this->VT_ptr = vt_ptr; }
  VTPtr(Self*) : VTPtrBase<Self>(&Self::VT_impl::VT) { }

  typename Self::VT_type* clone(Self*)
  {
    typename Self::VT_type* new_vt_ptr = new typename Self::VT_type(*this->VT_ptr);
    this->VT_ptr = new_vt_ptr;
    return new_vt_ptr;
  }
};

} // namespace utils
