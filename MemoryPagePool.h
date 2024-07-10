/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class MemoryPagePool.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
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

#include "macros.h"
#include "SimpleSegregatedStorage.h"
#include "debug.h"
#include <mutex>
#include <algorithm>
#include <unistd.h>

namespace utils {

namespace details {

// Helper class to provide memory_page_size().
struct MemoryPageSize
{
  static size_t memory_page_size()
  {
    static size_t const memory_page_size_ = sysconf(_SC_PAGE_SIZE);
    return memory_page_size_;
  }
};

} // namespace details

// A memory pool that returns fixed-size memory blocks allocated with std::aligned_alloc and aligned to memory_page_size.
//
class MemoryPagePool : public details::MemoryPageSize
{
 public:
  using blocks_t = unsigned int;

 private:
  utils::SimpleSegregatedStorage m_sss;
  size_t const m_block_size;            // The size of a block as returned by allocate(), in bytes.
  blocks_t m_pool_blocks;               // The total amount of allocated system memory, in blocks.
  blocks_t const m_minimum_chunk_size;  // The minimum size of internally allocated contiguous memory blocks, in blocks.
  blocks_t const m_maximum_chunk_size;  // The maximum size of internally allocated contiguous memory blocks, in blocks.
  std::vector<void*> m_chunks;          // All allocated chunks that were allocated with std::aligned_alloc.

 protected:
  virtual blocks_t default_minimum_chunk_size() { return 2; }
  virtual blocks_t default_maximum_chunk_size(blocks_t UNUSED_ARG(minimum_chunk_size)) { return 1024; }

 public:
  MemoryPagePool(size_t block_size,                     // The size of a block as returned by allocate(), in bytes; must be a multiple the memory page size.
                 blocks_t minimum_chunk_size = 0,       // A value of 0 will use the value returned by default_minimum_chunk_size().
                 blocks_t maximum_chunk_size = 0);      // A value of 0 will use the value returned by default_maximum_chunk_size(minimum_chunk_size).
  virtual ~MemoryPagePool()
  {
    DoutEntering(dc::notice, "MemoryPagePool::~MemoryPagePool() [" << this << "]");
    release();
  }

  void* allocate()
  {
    return m_sss.allocate([this](){
        // This runs in the critical area of utils::SimpleSegregatedStorage::m_add_block_mutex.
        blocks_t extra_blocks = std::clamp(m_pool_blocks, m_minimum_chunk_size, m_maximum_chunk_size);
        size_t extra_size = extra_blocks * m_block_size;
        void* chunk = std::aligned_alloc(memory_page_size(), extra_size);
        if (AI_UNLIKELY(chunk == nullptr))
          return false;
        m_sss.add_block(chunk, extra_size, m_block_size);
        m_pool_blocks += extra_blocks;
        m_chunks.push_back(chunk);
        return true;
    });
  }

  void deallocate(void* ptr)
  {
    m_sss.deallocate(ptr);
  }

  void release();

  size_t block_size() const { return m_block_size; }
  blocks_t pool_blocks() { std::scoped_lock<std::mutex> lock(m_sss.m_add_block_mutex); return m_pool_blocks; }
};

} // namespace utils
