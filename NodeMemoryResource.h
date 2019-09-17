// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class NodeMemoryResource.
//
// Copyright (C) 2019 Carlo Wood.
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
class NodeMemoryResource
{
 public:
  NodeMemoryResource(MemoryPagePool& mpp) : m_mpp(&mpp), m_block_size(0)
  {
    DoutEntering(dc::notice, "NodeMemoryResource::NodeMemoryResource({" << (void*)m_mpp << "}) [" << this << "]");
  }

  ~NodeMemoryResource()
  {
    DoutEntering(dc::notice, "NodeMemoryResource::~NodeMemoryResource() [" << this << "]");
  }

  void* allocate(size_t block_size)
  {
    //DoutEntering(dc::notice|continued_cf, "NodeMemoryResource::allocate(" << block_size << ") = ");
    if (AI_UNLIKELY(m_block_size == 0))
      m_block_size = block_size;
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

