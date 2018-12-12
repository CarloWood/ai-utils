#pragma once

template<class Self, class Base = void>
struct VTPtr
{
  using VT_type = typename Self::VT_type;
  using VT_impl = typename Self::VT_impl;
  VT_type const* VT_ptr = &VT_impl::VT;
  VT_type const* operator->() { return VT_ptr; }
  void set(Self* self, VT_type const* vt_ptr) { VT_ptr = vt_ptr; self->Base::VT_ptr.set(self, VT_ptr); }
  VTPtr(Self* self) : VT_ptr(&VT_impl::VT) { self->Base::VT_ptr.set(self, VT_ptr); }
};

template<class Self>
struct VTPtr<Self, void>
{
  using VT_type = typename Self::VT_type;
  using VT_impl = typename Self::VT_impl;
  VT_type const* VT_ptr = &VT_impl::VT;
  VT_type const* operator->() { return VT_ptr; }
  void set(Self*, VT_type const* vt_ptr) { VT_ptr = vt_ptr; }
  VTPtr(Self*) : VT_ptr(&VT_impl::VT) { }
};
