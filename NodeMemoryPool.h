// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class NodeMemoryPool.
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

#include "debug.h"
#include <vector>
#include <mutex>
#include <iosfwd>

namespace utils {
class NodeMemoryPool;
}

std::ostream& operator<<(std::ostream& os, utils::NodeMemoryPool const& pool);
inline void* operator new(std::size_t size, utils::NodeMemoryPool& pool);

namespace utils {

struct Begin;
struct FreeList;

// class NodeMemoryPool
//
// This memory pool is intended for fixed size allocations (ie, of the same object),
// one object at a time; where the size and type of the object are not known until
// the first allocation.
//
// The reason for that is that it is intended to be used with std::allocate_shared.
//
// Usage:
//
// utils::NodeMemoryPool pool(64); // Will allocate 64 objects at a time.
//
// utils::Allocator<MyObject, utils::NodeMemoryPool> allocator(pool);
// std::shared_ptr<MyObject> = std::allocate_shared<MyObject>(allocator, ...MyObject constructor arguments...);
//
// The Allocator can also be used to allocate objects of different types provided
// that their sizes are (about) the same. For example, if you want to allocate objects
// of size 32 and 30 bytes you could do:
//
// utils::Allocator<MyObject32, utils::NodeMemoryPool> allocator1(pool);
// utils::Allocator<MyObject30, utils::NodeMemoryPool> allocator2(pool);
//
// Just makes sure to FIRST allocate an object of the largest size; or
// make use of the pool constructor that sets the size in advance
// (this requires that you know the size in advance however):
//
// utils::NodeMemoryPool pool(64, 32); // Will allocate 64 objects of 32 bytes at a time.
//
// It is also possible to use this memory pool to replace heap allocation with new
// by adding an operator delete to an object and then using new with placement to
// create those objects. For example,
//
// class Foo {
//  public:
//   Foo(int);
//   void operator delete(void* ptr) { utils::NodeMemoryPool::static_free(ptr); }
// };
//
// and then create objects like
//
// utils::NodeMemoryPool pool(128, sizeof(Foo));
//
// Foo* foo = new(pool) Foo(42);        // Allocate memory from memory pool and construct object.
// delete foo;                          // Destruct object and return memory to the memory pool.
//
// NodeMemoryPool is thread-safe.

class NodeMemoryPool
{
 private:
  mutable std::mutex m_pool_mutex;      // Protects the pool against concurrent accesses.

  size_t const m_nchunks;               // The number of `m_size' sized chunks to allocate at once. Should always be larger than 0.
  FreeList* m_free_list;                // The next free chunk, or nullptr if there isn't any left.
  std::vector<Begin*> m_blocks;         // A list of all allocated blocks.
  size_t m_size;                        // The (fixed) size of a single chunk in bytes.
                                        // alloc() always returns a chunk of this size except the first time when m_free_list is still 0.
  size_t m_total_free;                  // The current total number of free chunks in the memory pool.

  friend void* ::operator new(std::size_t size, NodeMemoryPool& pool);
  void* alloc(size_t size);

 public:
  NodeMemoryPool(int nchunks, size_t chunk_size = 0) : m_nchunks(nchunks), m_free_list(nullptr), m_size(chunk_size), m_total_free(0) { }

  template<class Tp>
  Tp* malloc() { return static_cast<Tp*>(alloc(sizeof(Tp))); }

  void free(void* ptr);
  static void static_free(void* ptr);

  friend std::ostream& ::operator<<(std::ostream& os, NodeMemoryPool const& pool);
};

template<class Tp, class Mp>
struct Allocator
{
  Mp& m_memory_pool;

  using value_type = Tp;
  size_t max_size() const { return 1; }
  Tp* allocate(std::size_t n);
  void deallocate(Tp* p, std::size_t DEBUG_ONLY(n)) { ASSERT(n == 1); m_memory_pool.free(p); }
  Allocator(Mp& memory_pool) : m_memory_pool(memory_pool) { }
  template<class T> Allocator(Allocator<T, Mp> const& other) : m_memory_pool(other.m_memory_pool) { }
  template<class T> bool operator==(Allocator<T, Mp> const& other) { return &m_memory_pool == &other.m_memory_pool; }
  template<class T> bool operator!=(Allocator<T, Mp> const& other) { return &m_memory_pool != &other.m_memory_pool; }
};

template<class Tp, class Mp>
Tp* Allocator<Tp, Mp>::allocate(std::size_t DEBUG_ONLY(n))
{
  ASSERT(n == 1);
  return m_memory_pool.template malloc<Tp>();
}

} // namespace utils

inline void* operator new(std::size_t size, utils::NodeMemoryPool& pool) { return pool.alloc(size); }
