/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class NodeMemoryResource.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "utils/MemoryPagePool.h"
#include "utils/SimpleSegregatedStorage.h"
#include "debug.h"

namespace utils {

// class NodeMemoryResource
//
// A fixed size memory resource that uses a MemoryPagePool as upstream.
// The block size is determined during runtime from the first allocation,
// which allows it to be used for allocators that allocate unknown types.
//
// Usage example:
//
//   utils::MemoryPagePool mpp(0x8000);                 // Serves chunks of 32 kB.
//   utils::NodeMemoryResource nmr(mpp);                // Serves chunks of unknown but fixed size (512 bytes in the case of a deque).
//   DequePoolAllocator<AIStatefulTask*> alloc(nmr);    // Wrapper around a pointer to utils::NodeMemoryResource, providing an allocator interface.
//   std::deque<AIStatefulTask*, decltype(alloc)> test_deque(alloc);
//
// Note: it is possible to specify a block size upon construction (which obviously must be
// larger or equal to the actual (largest) block size that will be allocated).
//
class NodeMemoryResource
{
 public:
  // Create an uninitialized NodeMemoryResource. Call init() to initialize it.
  NodeMemoryResource() : m_mpp(nullptr), m_block_size(0) { }

  // Create an initialized NodeMemoryResource.
  NodeMemoryResource(MemoryPagePool& mpp, size_t block_size = 0) : m_mpp(&mpp), m_block_size(block_size)
  {
    DoutEntering(dc::notice, "NodeMemoryResource::NodeMemoryResource({" << (void*)m_mpp << "}, " << block_size << ") [" << this << "]");
  }

  // Destructor.
  ~NodeMemoryResource()
  {
    DoutEntering(dc::notice(m_mpp), "NodeMemoryResource::~NodeMemoryResource() [" << this << "]");
  }

  // Late initialization.
  void init(MemoryPagePool* mpp_ptr, size_t block_size = 0)
  {
    // A NodeMemoryResource object may only be initialized once.
    ASSERT(m_mpp == nullptr);
    m_mpp = mpp_ptr;
    m_block_size = block_size;
    Dout(dc::notice(block_size > 0), "NodeMemoryResource::m_block_size using [" << m_mpp << "] set to " << block_size << " [" << this << "]");
  }

  void* allocate(size_t block_size)
  {
    //DoutEntering(dc::notice|continued_cf, "NodeMemoryResource::allocate(" << block_size << ") = ");
    if (AI_UNLIKELY(m_block_size == 0))
    {
      static std::mutex m;
      std::lock_guard<std::mutex> guard(m);
      // Call NodeMemoryResource::init before using a default constructed NodeMemoryResource.
      // If this is inside a call to AIStatefulTaskMutex::lock then you probably forgot to create
      // a statefultask::DefaultMemoryPagePool object at the top of main. Go read the documentation
      // at the top of statefultask/DefaultMemoryPagePool.h.
      ASSERT(m_mpp != nullptr);
      m_block_size = block_size;
      Dout(dc::notice, "NodeMemoryResource::m_block_size using [" << m_mpp << "] set to " << block_size << " [" << this << "]");
    }
#ifdef CWDEBUG
    else
      ASSERT(block_size <= m_block_size);
#endif
    void* ptr = m_sss.allocate([this](){
          void* chunk = m_mpp->allocate();
          if (!chunk)
            return false;
          m_sss.add_block(chunk, m_mpp->block_size(), m_block_size);
          return true;
        });
    //Dout(dc::finish, ptr);
    return ptr;
  }

  void deallocate(void* ptr)
  {
    //DoutEntering(dc::notice, "NodeMemoryResource::deallocate(" << ptr << ")");
    m_sss.deallocate(ptr);
  }

 private:
  MemoryPagePool* m_mpp;
  SimpleSegregatedStorage m_sss;
  size_t m_block_size;
};

} // namespace utils

