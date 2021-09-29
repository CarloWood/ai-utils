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
//   utils::DequeAllocator<AIStatefulTask*> alloc(nmr); // Wrapper around a pointer to utils::NodeMemoryResource, providing an allocator interface.
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
    size_t stored_block_size = m_block_size.load(std::memory_order_relaxed);
    if (AI_UNLIKELY(stored_block_size == 0))
    {
      // No mutex is required here; it is not allowed to have a race condition between
      // two different block_size's. If different block sizes are used, then the largest
      // block_size must be used first, as in: the call to allocate(largest_block_size)
      // MUST already have returned before a call to allocate(smaller_block_size) may
      // happen. It is the responsibility of the user to make sure this is the case.
      //
      // Therefore we can assume here that any race conditions between multiple threads
      // calling this function while m_block_size is still 0 happen with the same value
      // of block_size.

      // Call NodeMemoryResource::init before using a default constructed NodeMemoryResource.
      // If this is inside a call to AIStatefulTaskMutex::lock then you probably forgot to create
      // a statefultask::DefaultMemoryPagePool object at the top of main. Go read the documentation
      // at the top of statefultask/DefaultMemoryPagePool.h.
      //
      // If this is inside a call to utils::DequeMemoryResource::allocate then you forgot to
      // construct a utils::DequeMemoryResource::Initialization object at the top of main.
      ASSERT(m_mpp != nullptr);
      m_block_size.store(block_size, std::memory_order_relaxed);
      stored_block_size = block_size;
      Dout(dc::notice, "NodeMemoryResource::m_block_size using [" << m_mpp << "] set to " << block_size << " [" << this << "]");
    }
#ifdef CWDEBUG
    else
      ASSERT(block_size <= stored_block_size);
#endif
    void* ptr = m_sss.allocate([this, stored_block_size](){
          void* chunk = m_mpp->allocate();
          if (!chunk)
            return false;
          m_sss.add_block(chunk, m_mpp->block_size(), stored_block_size);
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
  std::atomic<size_t> m_block_size;
};

} // namespace utils

