// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class NodeMemoryPool
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

#include "sys.h"
#include "debug.h"
#include "NodeMemoryPool.h"
#include "macros.h"
#include "is_power_of_two.h"
#include <cstdlib>
#include <cstddef>
#include <algorithm>

namespace utils {

union Next
{
  size_t n;                     // This must have the same size as a pointer, so that setting this to zero causes ptr == nullptr.
  FreeList* ptr;
};

struct FreeList
{
  ssize_t* free;                // Points to Begin::free of the current block.
  Next m_next;                  // m_next.ptr either points to the next free chunk in the free list, or is nullptr when there are no free chunks left.
};

struct Allocated
{
  ssize_t* free;
  char data[];                  // m_size bytes of data.
};

union Chunk
{
  FreeList free_list;
  Allocated allocated;
};

struct Begin
{
  ssize_t free;                 // Each allocated memory block (of m_nchunk chunks of size m_size each) begins with a size_t that counts the number of free chunks in the block.
  NodeMemoryPool* pool;         // A pointer to the pool object to support overloading operator delete for objects.
  Chunk first_chunk;            // Subsequently there is a Chunk object, of which this is the first, every offsetof(Allocated, data) + m_size bytes (aka the real size of "Allocated",
                                // aka the real size of Chunk, where sizeof(FreeList) needs to be less than or equal the size of Allocated. In other words m_size >= sizeof(Next)).
};

static_assert(offsetof(FreeList, m_next) == offsetof(Allocated, data));

#ifdef CWDEBUG
static_assert(alignof(Chunk) == alignof(size_t));    // Because we shift all Chunk`s by a size_t (technically, alignof(size_t) should be a multiple of alignof(Chunk)).
static_assert(is_power_of_two(alignof(Chunk)));
static constexpr size_t chunk_align_mask = alignof(Chunk) - 1;
#endif

void* NodeMemoryPool::alloc(size_t size)
{
  std::unique_lock<std::mutex> lock(m_free_list_mutex);
  FreeList* ptr = m_free_list;
  if (AI_UNLIKELY(!ptr))
  {
    if (AI_UNLIKELY(!m_size))
      m_size = size;    // If m_size wasn't initialized yet, set it to the size of the first allocation.
    // m_size must be greater or equal sizeof(Next), and a multiple of alignof(Chunk).
    ASSERT(m_size >= sizeof(Next) && (m_size & chunk_align_mask) == 0);
    // Allocate space for Begin::free plus Begin::pool followed by m_nchunks of m_size (offsetof(Allocated, data) + m_size) (the real size of Allocated).
    Begin* begin = static_cast<Begin*>(std::malloc(offsetof(Begin, first_chunk) + m_nchunks * (offsetof(Allocated, data) + m_size)));
    begin->pool = this;
    ptr = m_free_list = &begin->first_chunk.free_list;
    ptr->m_next.n = m_nchunks - 1;
    ptr->free = &begin->free;
    *ptr->free = m_nchunks;
    std::unique_lock<std::mutex> lock(m_blocks_mutex);
    m_blocks.push_back(begin);
    m_total_free += m_nchunks;
  }
  // size must fit.
  ASSERT(size <= m_size);
  if (AI_UNLIKELY(ptr->m_next.n < m_nchunks && ptr->m_next.ptr))
  {
    size_t n = ptr->m_next.n;
    ptr->m_next.ptr = reinterpret_cast<FreeList*>(reinterpret_cast<char*>(ptr) + m_size + offsetof(FreeList, m_next));
    ASSERT(ptr->m_next.n >= m_nchunks); // Smaller values are used as 'magic' values.
    ptr->m_next.ptr->m_next.n = n - 1;
    ptr->m_next.ptr->free = ptr->free;
  }
  m_free_list = ptr->m_next.ptr;
  --*ptr->free;
  --m_total_free;
  ASSERT(*ptr->free >= 0);
  return reinterpret_cast<Chunk*>(ptr)->allocated.data;
}

void NodeMemoryPool::free(void* p)
{
  // Interpret the pointer p as pointing to Chunk::allocated::data and reinterpret/convert it to a pointer to Chunk::free_list.
  FreeList* ptr = reinterpret_cast<FreeList*>(reinterpret_cast<char*>(p) - offsetof(Allocated, data));
  std::unique_lock<std::mutex> lock(m_free_list_mutex);
  ptr->m_next.ptr = m_free_list;
  m_free_list = ptr;
  ++*ptr->free;
  ++m_total_free;
  ASSERT(*ptr->free <= (ssize_t)m_nchunks);
  if (AI_UNLIKELY(*ptr->free == (ssize_t)m_nchunks) && m_total_free >= 2 * m_nchunks)
  {
    // The last chunk of this block was freed; delete it.
    // Find begin and end of the block.
    char* const begin = reinterpret_cast<char*>(ptr->free);
    char* const end = begin + offsetof(Begin, first_chunk) + m_nchunks * (offsetof(Allocated, data) + m_size);
    for (FreeList** fpp = &m_free_list; reinterpret_cast<size_t>(*fpp) >= m_nchunks; fpp = &(*fpp)->m_next.ptr)
    {
      while (begin <= reinterpret_cast<char*>(*fpp) && reinterpret_cast<char*>(*fpp) < end)
        *fpp = (*fpp)->m_next.ptr;
      if (reinterpret_cast<size_t>(*fpp) < m_nchunks)
        break;
    }
    m_total_free -= m_nchunks;
    std::free(begin);
    std::unique_lock<std::mutex> lock(m_blocks_mutex);
    m_blocks.erase(std::remove(m_blocks.begin(), m_blocks.end(), reinterpret_cast<Begin*>(begin)));
  }
}

//static
void NodeMemoryPool::static_free(void* ptr)
{
  Allocated* allocated = reinterpret_cast<Allocated*>(reinterpret_cast<char*>(ptr) - offsetof(Allocated, data));
  NodeMemoryPool* self = reinterpret_cast<Begin*>(allocated->free)->pool;
  self->free(ptr);
}

} // namespace utils

std::ostream& operator<<(std::ostream& os, utils::NodeMemoryPool const& pool)
{
  std::unique_lock<std::mutex> lock1(pool.m_free_list_mutex);
  std::unique_lock<std::mutex> lock2(pool.m_blocks_mutex);
  size_t allocated_size = (offsetof(utils::Begin, first_chunk) + pool.m_nchunks * (offsetof(utils::Allocated, data) + pool.m_size)) * pool.m_blocks.size();
  size_t num_chunks = pool.m_nchunks * pool.m_blocks.size();
  size_t num_free_chunks = 0;
  for (utils::Begin* begin : pool.m_blocks)
    num_free_chunks += begin->free;
  ASSERT(num_free_chunks == pool.m_total_free);
  os << "NodeMemoryPool stats: node size: " << pool.m_size << "; allocated size: " << allocated_size <<
      "; total/used/free: " << num_chunks << '/' << (num_chunks - num_free_chunks) << '/' << num_free_chunks;
  return os;
}
