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

#include <vector>
#include <mutex>
#include <iosfwd>

namespace utils {
class NodeMemoryPool;
}

std::ostream& operator<<(std::ostream& os, utils::NodeMemoryPool const& pool);

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
// NodeMemoryPool pool(64); // Will allocate 64 objects at a time.
// std::shared_ptr<MyObject> = std::allocate_shared<MyObject>(pool, ...MyObject constructor arguments...);
//
class NodeMemoryPool
{
 private:
  size_t const m_nchunks;               // The number of `size' sized chunks to allocate at once. Should always be larger than 0.
  mutable std::mutex m_free_list_mutex;
  FreeList* m_free_list;                // The next free chunk, or nullptr if there isn't any left.
  mutable std::mutex m_blocks_mutex;
  std::vector<Begin*> m_blocks;         // A list of all allocated blocks.
  size_t m_size;
  size_t m_total_free;

  void* alloc(size_t size);

 public:
  NodeMemoryPool(int nchunks) : m_nchunks(nchunks), m_free_list(nullptr), m_total_free(0) { }

  template<class Tp>
  Tp* malloc() { return static_cast<Tp*>(alloc(sizeof(Tp))); }

  void free(void* ptr);

  friend std::ostream& ::operator<<(std::ostream& os, NodeMemoryPool const& pool);
};

} // namespace utils
