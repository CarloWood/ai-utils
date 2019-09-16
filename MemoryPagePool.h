// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class MemoryPagePool.
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

#include "macros.h"
#include "SimpleSegregatedStorage.h"
#include "log2.h"
#include "nearest_power_of_two.h"
#include "debug.h"
#include <mutex>
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

// A memory pool that returns fixed-size memory blocks of BLOCK_SIZE bytes
// allocated with std::aligned_alloc and aligned to memory_page_size.
//
// BLOCK_SIZE must be a multiple the memory page size.
//
template<size_t BLOCK_SIZE>
class MemoryPagePool : public details::MemoryPageSize
{
  // We can't know what the memory page size will be during compile time, but it is almost
  // certainly not going to be smaller than 4 kB, so add this as a compile time sanity check.
  // The constructor contains a precise, runtime check.
  static_assert(BLOCK_SIZE % 0x1000 == 0 && BLOCK_SIZE > 0, "BLOCK_SIZE must be a multiple of memory_page_size, larger than 0.");

 public:
  using blocks_t = unsigned int;

  std::mutex m_mutex;                   // Protects the member variables below.
  utils::SimpleSegregatedStorage m_sss;
  blocks_t m_pool_size;                 // The total amount of allocated system memory, in blocks.
  blocks_t const m_minimum_chunk_size;  // The minimum size of internally allocated contiguous memory blocks, in blocks.
  blocks_t const m_maximum_chunk_size;  // The maximum size of internally allocated contiguous memory blocks, in blocks.
  std::vector<void*> m_chunks;          // All allocated chunks that were allocated with std::aligned_alloc.

 protected:
  virtual blocks_t default_minimum_chunk_size() { return 2; }
  virtual blocks_t default_maximum_chunk_size(blocks_t UNUSED_ARG(minimum_chunk_size)) { return 1024; }

 public:
  MemoryPagePool(blocks_t minimum_chunk_size = 0,       // A value of 0 will use the value returned by default_minimum_chunk_size().
                 blocks_t maximum_chunk_size = 0);      // A value of 0 will use the value returned by default_maximum_chunk_size(minimum_chunk_size).
  ~MemoryPagePool() { release(); }

  void* allocate()
  {
    return m_sss.allocate([this](){
        // This run in the critical area of utils::SimpleSegregatedStorage::m_add_block_mutex.
        blocks_t extra_blocks = std::clamp(m_pool_size, m_minimum_chunk_size, m_maximum_chunk_size);
        size_t extra_size = extra_blocks * BLOCK_SIZE;
        void* chunk = std::aligned_alloc(memory_page_size(), extra_size);
        if (AI_UNLIKELY(chunk == nullptr))
          return false;
        m_sss.add_block(chunk, extra_size, BLOCK_SIZE);
        m_pool_size += extra_blocks;
        m_chunks.push_back(chunk);
        return true;
    });
  }

  void deallocate(void* ptr)
  {
    m_sss.deallocate(ptr);
  }

  void release();

  static constexpr size_t block_size() { return BLOCK_SIZE; }
};

template<size_t BLOCK_SIZE>
MemoryPagePool<BLOCK_SIZE>::MemoryPagePool(blocks_t minimum_chunk_size, blocks_t maximum_chunk_size) :
  m_pool_size(0),
  m_minimum_chunk_size(minimum_chunk_size ? minimum_chunk_size : default_minimum_chunk_size()),
  m_maximum_chunk_size(maximum_chunk_size ? maximum_chunk_size : default_maximum_chunk_size(m_minimum_chunk_size))
{
  // BLOCK_SIZE must be a multiple of memory_page_size (and larger than 0).
  ASSERT(BLOCK_SIZE % memory_page_size() == 0);
  // minimum_chunk_size must be larger or equal than 1.
  ASSERT(m_minimum_chunk_size >= 1);
  // maximum_chunk_size must be larger or equal than minimum_chunk_size.
  ASSERT(m_maximum_chunk_size >= m_minimum_chunk_size);

  DoutEntering(dc::notice, "MemoryPagePool<" << BLOCK_SIZE << ">::MemoryPagePool(" << minimum_chunk_size << ", " << maximum_chunk_size << ")");

  // This capacity is enough for allocating twice the maximum_chunk_size of memory (and then rounded up to the nearest power of two).
  m_chunks.reserve(nearest_power_of_two(1 + log2(m_maximum_chunk_size)));
  Dout(dc::notice, "The block size (" << BLOCK_SIZE << " bytes) is " << (BLOCK_SIZE / memory_page_size()) << " times the memory page size on this machine.");
  Dout(dc::notice, "The capacity of m_chunks is " << m_chunks.capacity() << '.');
}

template<size_t BLOCK_SIZE>
void MemoryPagePool<BLOCK_SIZE>::release()
{
  // Wink out any remaining allocations.
  for (auto ptr : m_chunks)
    std::free(ptr);
}

} // namespace utils
