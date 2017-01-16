/**
 * @file
 * @brief Definition of class AIRefCount.
 *
 * Copyright (C) 2017 Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <atomic>
#include <boost/intrusive_ptr.hpp>

// Usage:
//
// class MyClass : public AIRefCount {
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

class AIRefCount {
  private:
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

  protected:
    AIRefCount() : m_count(0) { }
    AIRefCount(AIRefCount const&) : m_count(0) { }
    AIRefCount& operator=(AIRefCount const&) { return *this; }
    virtual ~AIRefCount() { }
    void swap(AIRefCount&) { }

  public:
    bool unique() const { return std::atomic_load_explicit(&m_count, std::memory_order_relaxed) == 1; }
    int ref_count() const { return m_count; }
};
