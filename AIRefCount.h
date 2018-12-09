// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class AIRefCount.
//
// Copyright (C) 2017 Carlo Wood.
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

#include <atomic>
#include <boost/intrusive_ptr.hpp>
#include "debug.h"

// Usage:
//
// class MyClass : public AIRefCount
// {
// };
//
// int main()
// {
//   boost::intrusive_ptr<MyClass> my_class = new MyClass;      // m_count = 1
//   {
//     MyClass* p = my_class.get();
//     boost::intrusive_ptr<MyClass> ip(p);                     // m_count = 2
//   }                                                          // m_count = 1
//   my_class->print();
// }                                                            // m_count = 0 --> destruction of MyClass.

class AIRefCount
{
 private:
#ifdef CWDEBUG
  static constexpr int s_deleted = -0x6de1e7ed;                 // A negative magic number.
#endif
  mutable std::atomic<int> m_count;

 public:
  friend void intrusive_ptr_add_ref(AIRefCount const* ptr)
  {
    ptr->m_count.fetch_add(1, std::memory_order_relaxed);
  }

  friend void intrusive_ptr_release(AIRefCount const* ptr)
  {
    if (ptr->m_count.fetch_sub(1, std::memory_order_release) == 1)
    {
      std::atomic_thread_fence(std::memory_order_acquire);
      delete ptr;
    }
  }

  void inhibit_deletion(DEBUG_ONLY(bool can_cause_immediate_allow_deletion = true)) const
  {
    DEBUG_ONLY(int count =) m_count.fetch_add(1, std::memory_order_relaxed);
    // Because m_count is overwritten with s_deleted upon destruction when CWDEBUG is defined, a reference count of
    // zero means that this object is probably still being constructed and wasn't passed to a boost::intrusive_ptr _yet_.
    // If under those circumstances an intrusive_ptr_add_ref/intrusive_ptr_release pair,
    // that is merely intended to temporarily inhibit deletion, could inadvertently delete
    // the object while it is still being constructed (at least, before we even got
    // the chance to pass it to the first boost::intrusive_ptr).
    // This test can help in detecting that it was not a good idea to call inhibit_deletion
    // if that can result in a subsequent call to allow_deletion() before that first
    // boost::intrusive_ptr is created. If that is not possible then you can pass
    // false to this function.
    //
    // Normally, when this assert fires you should find the function that was
    // called from the constructor of the derived class that caused inhibit_deletion
    // to be called, and delay that call to after you have a boost::intrusive_ptr.
    //
    // For example,
    //
    // struct A : public AIRefCount {
    //   void f() { inhibit_deletion(); }
    //   A() { f(); }
    // };
    //
    // boost::intrusive_ptr<A> p = new A;
    //
    // Should become,
    //
    // struct A : public AIRefCount {
    //   void f() { inhibit_deletion(); }
    //   A() { }
    // };
    //
    // boost::intrusive_ptr<A> p = new A;
    // p->f();
    ASSERT(!can_cause_immediate_allow_deletion || count > 0);
  }

  void allow_deletion() const
  {
    if (m_count.fetch_sub(1, std::memory_order_release) == 1)
    {
      std::atomic_thread_fence(std::memory_order_acquire);
      delete this;
    }
  }

 private:
  // You should use inhibit_deletion / allow_deletion.
  void intrusive_ptr_add_ref(AIRefCount const*);
  void intrusive_ptr_release(AIRefCount const*);

 protected:
  AIRefCount() : m_count(0) { }
  AIRefCount(AIRefCount const&) : m_count(0) { }
  virtual ~AIRefCount() { Debug(m_count = s_deleted); }
  AIRefCount& operator=(AIRefCount const&) { return *this; }
  void swap(AIRefCount&) { }

 public:
  bool unique() const { return std::atomic_load_explicit(&m_count, std::memory_order_relaxed) == 1; }
  int ref_count() const { return m_count; }
#ifdef CWDEBUG
  // Pretty unreliable, but sometimes useful.
  bool is_destructed() const { return std::atomic_load_explicit(&m_count, std::memory_order_relaxed) == s_deleted; }
#endif
};
