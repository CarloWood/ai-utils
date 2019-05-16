// ai-utils -- C++ Core utilities
//
//! @file
//! @brief A FIFO buffer for trivially copyable types.
//
// Copyright (C) 2017  Carlo Wood.
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
//
// CHANGELOG
//   and additional copyright holders.
//
//   2017/04/25
//   - Initial version, written by Carlo Wood.
//
//   2018/01/02
//   - Changed license to GPL-3.

#pragma once

// Lock-free ring buffer for trivially copyable objects, to be written and
// read (using std::memcpy) in chunks of (a fixed) T_per_chunk objects at a time.
//
// In most cases T will be a pointer; but it can be anything else that
// is trivially copyable (can be moved around using memcpy);
//
//  _ _ _ _________________________________________ _ _ _ _
// G | HH | AA | BB | CC | DD | EE | FF | GG | HH | AA | BB
//  - - - ----------------------------------------- - - - -
//        ^         ^         ^         ^         ^
//     end/begin  m_head   m_tail   m_readptr  end/begin
//     m_buffer
//
// The pointers, m_head, m_tail and m_readptr, move from left to right; but cannot overtake eachother.
// If they pass the end, they wrap around to the beginning again. m_tail can become equal to m_readptr,
// which can become equal to m_head (buffer empty); but m_head can't advance beyond one place before
// m_tail at which point the buffer is full.
//
// Next to be overwritten: CC ; Next returned to be read: GG ; Still waiting to be copied and popped: EE and FF.
//
// push()/push_zero()
// If the distance between head and tail is at least two chunks (it is precisely two in the image), then a
// call to push() copies one chunk to m_head and then advances m_head one position (so that it will never
// become equal to m_tail). Note that when m_head equals m_tail, then the buffer is empty.
// push_zero() does the same but writes zeroes using memset instead of copying data provided by the caller.
// Both Return true if the operation succeeded, false if the buffer is full.
//
// pop()
// Return the current position of m_tail and advance it one chunk. Because push() never overwrites the
// chunk left of m_tail it is still safe to read that chunk until the next call to pop() (this class only
// allows ONE producer thread and ONE consumer thread!) Returns nullptr if the buffer is empty.
//
// read()
// Return the current position of m_readptr and advance it one chunk. Returns nullptr if there is nothing
// more to read. This function can be used to peek if there is anything left without actually reading it
// (when reading from pop()), or to get the next chunk without destructing the data that the pointer
// returned last by pop() (or read()) is pointing to.
//
// reset_readptr()
// Move m_readptr back to where m_tail is.
//
// clear()
// Empty the buffer (sets m_tail and m_readptr to where m_head is).
//
template <int T_per_chunk, typename T>
class AIFIFOBuffer
{
  // This buffer uses std::memcpy to move the objects around.
  static_assert(std::is_trivially_copyable<T>::value, "AIFIFOBuffer requires a trivially copyable type.");
  // If this fails than reallocate_buffer needs to be changed to allocate memory aligned to 'alignment'.
  static_assert(alignof(T) <= alignof(std::max_align_t), "AIFIFOBuffer requires objects with fundamental alignment.");

 public:
  static constexpr size_t alignment           = alignof(T);               // Buffer alignment (as well as the alignment of every other pointer we deal with).
  static constexpr size_t chunk_size          = T_per_chunk * sizeof(T);  // Amount of bytes we read/write at a time (a chunk).
  static constexpr intptr_t objects_per_chunk = T_per_chunk;              // The number of objects T that are read and written at a time.

 private:
  intptr_t m_capacity;     // Total number of objects of type T in the buffer.
  union {
    T* m_buffer;           // Buffer start.
    typename std::aligned_storage<sizeof(T), alignof(T)>::type* m_storage;
  };
  std::atomic<T*> m_head;  // Write position in circular buffer.
  T* m_readptr;            // Non-destructive read position in circular buffer.
  std::atomic<T*> m_tail;  // Read position in circular buffer.

 private:
  // Return ptr advanced one chunk.
  T* increment(T* ptr) const
  {
    intptr_t next_ptr = ptr - m_buffer + objects_per_chunk;
    return m_buffer + (next_ptr == m_capacity ? 0 : next_ptr);
  }

 public:
  // Construct a buffer with zero capacity. Call reallocate_buffer to finish initialization.
  AIFIFOBuffer() : m_capacity(0), m_buffer(nullptr), m_head(nullptr), m_readptr(nullptr), m_tail(nullptr) {}
  // Construct a buffer of nchunk chunks (each of n objects).
  AIFIFOBuffer(int nchunks) : m_buffer(nullptr) { reallocate_buffer(nchunks); }
  // Destructor.
  virtual ~AIFIFOBuffer() { if (m_storage) delete [] m_storage; }

  //-------------------------------------------------------------------------
  // Producer thread.

  // Copy one chunk from `in' to m_head and advance head. Returns true if the operation succeeded, false if the buffer is full.
  bool push(T const* in)
  {
    auto const current_head = m_head.load(std::memory_order_relaxed);
    auto const next_head    = increment(current_head);

    if (next_head != m_tail.load(std::memory_order_acquire))  // Otherwise the buffer would appear empty after the m_head.store below,
                                                              // and we'd be writing over data that possibly still needs to be read
                                                              // by the consumer (that was returned by pop()).
    {
      std::memcpy(current_head, in, chunk_size);
      m_head.store(next_head, std::memory_order_release);
      return true;
    }

    return false;  // Full queue.
  }

  // Same as the above but writes zero's.
  bool push_zero()
  {
    auto const current_head = m_head.load(std::memory_order_relaxed);
    auto const next_head    = increment(current_head);

    if (next_head != m_tail.load(std::memory_order_acquire))  // Otherwise the buffer would appear empty after the m_head.store below,
                                                              // and we'd be writing over data that possibly still needs to be read
                                                              // by the consumer (that was returned by pop()).
    {
      std::memset(current_head, 0, chunk_size);
      m_head.store(next_head, std::memory_order_release);
      return true;
    }

    return false;  // Full queue.
  }

  //-------------------------------------------------------------------------
  // Consumer thread.

  // Return m_tail and advance it, possibly also advancing m_readptr. Returns nullptr if the buffer is empty.
  T* pop()
  {
    auto const current_tail = m_tail.load(std::memory_order_relaxed);
    if (current_tail == m_head.load(std::memory_order_acquire)) return nullptr;  // Empty queue.

    auto const next_tail                     = increment(current_tail);
    if (current_tail == m_readptr) m_readptr = next_tail;
    m_tail.store(next_tail, std::memory_order_release);
    return current_tail;
  }

  // Return m_readptr and advance it. Returns nullptr if the read pointer is at the end of the buffer.
  T* read()
  {
    auto current_ptr = m_readptr;
    if (current_ptr == m_head.load(std::memory_order_acquire)) return nullptr;  // At end.

    m_readptr = increment(current_ptr);
    return current_ptr;
  }

  // Reset the read pointer to the beginning of the recorded data in the buffer.
  void reset_readptr()
  {
    m_readptr = m_tail.load(std::memory_order_relaxed);
  }

  // Clear the buffer.
  //
  // There is no way to clear the buffer in a thread-safe way from the producer (or any other thread but the consumer thread).
  // However, any thread may call this function to clear the buffer if it is known that the consumer thread is not using the
  // buffer at that moment. This is the equivalent of atomically calling read() until it returns nullptr and then atomically
  // calling pop() until it returns nullptr.
  void clear()
  {
    auto const current_head = m_head.load(std::memory_order_relaxed);
    m_readptr               = current_head;
    m_tail.store(current_head, std::memory_order_release);
  }

  //-------------------------------------------------------------------------

  // Return value    :  true                                false
  // Producer thread :  Is empty/at_end.                    Is, or was recently, non-empty.
  // Consumer thread :  Is, or was recently, empty/at_end.  Is non-empty.
  // Other threads   :             meaningless (don't call this)
  bool empty() const
  {
    auto const current_head = m_head.load(std::memory_order_relaxed);
    return current_head == m_tail.load(std::memory_order_relaxed);
  }

  // Return value    :  true                                false
  // Producer thread :  Is at end.                          Is, or was recently, not at end.
  // Consumer thread :  Is, or was recently, at end.        Is not at end.
  // Other threads   :             meaningless (don't call this)
  bool at_end() const
  {
    auto const current_head = m_head.load(std::memory_order_relaxed);
    return current_head == m_readptr;
  }

  // Return value    :  true                                false
  // Producer thread :  Is, or was recently, full.          Is not full.
  // Consumer thread :  Is full.                            Is, or was recently, not full.
  // Other threads   :             meaningless (don't call this)
  bool full() const
  {
    auto const current_tail = m_tail.load(std::memory_order_relaxed);
    auto const next_head    = increment(m_head.load(std::memory_order_relaxed));
    return next_head == current_tail;
  }

  bool is_lock_free() const { return m_head.is_lock_free() && m_tail.is_lock_free(); }
  void reallocate_buffer(int nchunks);
};

// This may only be used to resize a buffer that is not in use at the moment.
template <int T_per_chunk, typename T>
void AIFIFOBuffer<T_per_chunk, T>::reallocate_buffer(int nchunks)
{
  m_capacity = T_per_chunk * nchunks;
  // The following is safe because the buffer isn't used at the moment.
  if (m_storage)
    delete [] m_storage;
  m_storage = new typename std::aligned_storage<sizeof(T), alignof(T)>::type [m_capacity];
  Dout(dc::notice, "Allocated AIFIFOBuffer buffer at " << m_buffer << " till " << &m_buffer[m_capacity]);
  m_head = m_buffer;
  clear();
}
