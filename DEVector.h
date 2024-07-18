#pragma once

#include "malloc_size.h"
#include <new>
#include <type_traits>
#include <cstdlib>
#include <algorithm>
#include "debug.h"
#ifdef CWDEBUG
#include "has_print_on.h"
#endif

namespace utils {
using utils::has_print_on::operator<<;

// DEVector
//
// Double-Ended vector.
//
// Allocates memory when needed, which is never freed again, keeping room
// both left and right of the stored data; so that adding new element at
// the front is equivalent to adding new elements at the end.
//
// Inserting elements in the middle will move either to the left or the
// right, which ever needs less elements to be moved.
//
// Reallocations are roughly double the current size of the vector (the
// number of stored elements), so that if this vector is used as a queue
// were elements are pushed to the front and popped from the back,
// then no reallocation happens: the data IS however moved to the middle
// again every now and then.
template<typename T, size_t initial_size = 8>
class DEVector
{
 public:
  using iterator = T*;
  using const_iterator = T const*;
  using reference = T&;
  using const_reference = T const&;

 private:
  T* buffer_{nullptr};
  size_t capacity_{0};
  size_t size_{0};
  size_t zero_index_{0};

 private:
  constexpr T* allocate_buffer(size_t min_capacity);
  constexpr void reallocate(size_t front_cap, size_t back_cap);

 public:
  constexpr DEVector() = default;
  constexpr ~DEVector() { std::free(buffer_); }

  constexpr DEVector(DEVector&& orig)
  {
    std::memcpy(this, &orig, sizeof(*this));
    orig.buffer_ = nullptr;     // Allow destruction.
    orig.capacity_ = 0;         // Alloc calling clear(), which then results in a state equal to being default constructed.
  }

  constexpr DEVector(DEVector const&);

  constexpr size_t size() const { return size_; }

  constexpr void clear() { size_ = 0; zero_index_ = capacity_ / 2; }
  constexpr bool empty() const { return size_ == 0; }

  constexpr const_reference operator[](size_t pos) const { return buffer_[zero_index_ + pos]; }
  constexpr reference operator[](size_t pos) { return buffer_[zero_index_ + pos]; }

  constexpr const_reference back() const { return buffer_[zero_index_ + size_ - 1]; }
  constexpr reference back() { return buffer_[zero_index_ + size_ - 1]; }

  constexpr const_reference front() const { return buffer_[zero_index_]; }
  constexpr reference front() { return buffer_[zero_index_]; }

  constexpr const_iterator begin() const { return buffer_ + zero_index_; }
  constexpr iterator begin() { return buffer_ + zero_index_; }

  constexpr const_iterator end() const { return buffer_ + zero_index_ + size_; }
  constexpr iterator end() { return buffer_ + zero_index_ + size_; }

  constexpr void push_back(T const& value)
  {
    if (zero_index_ + size_ == capacity_)
      reallocate(0, 1);
    ++size_;
    ASSERT(zero_index_ + size_ <= capacity_);
    buffer_[zero_index_ + size_ - 1] = value;
  }
  constexpr void push_back(T&& value)
  {
    if (zero_index_ + size_ == capacity_)
      reallocate(0, 1);
    ++size_;
    ASSERT(zero_index_ + size_ <= capacity_);
    buffer_[zero_index_ + size_ - 1] = std::move(value);
  }

  template<typename... Args>
  constexpr reference emplace_back(Args&&... args)
  {
    if (zero_index_ + size_ == capacity_)
      reallocate(0, 1);
    ++size_;
    ASSERT(zero_index_ + size_ <= capacity_);
    return *new (buffer_ + zero_index_ + size_ - 1) T(std::forward<Args>(args)...);
  }

  constexpr void push_front(T const& value)
  {
    if (zero_index_ == 0)
      reallocate(1, 0);
    ASSERT(zero_index_ > 0);
    --zero_index_;
    ++size_;
    buffer_[zero_index_] = value;
  }
  constexpr void push_front(T&& value)
  {
    if (zero_index_ == 0)
      reallocate(1, 0);
    ASSERT(zero_index_ > 0);
    --zero_index_;
    ++size_;
    buffer_[zero_index_] = std::move(value);
  }

  template<typename... Args>
  constexpr reference emplace_front(Args&&... args)
  {
    if (zero_index_ == 0)
      reallocate(1, 0);
    ASSERT(zero_index_ > 0);
    --zero_index_;
    ++size_;
    return *new (buffer_ + zero_index_) T(std::forward<Args>(args)...);
  }

  //constexpr iterator insert(const_iterator pos, T const& value);
  //constexpr iterator insert(const_iterator pos, T&& value);

  template<typename... Args>
  constexpr iterator emplace(const_iterator pos, Args&&... args);

#ifdef CWDEBUG
  void print_on(std::ostream& os) const
  {
    os << "{<" << zero_index_ << ">";
    for (int i = zero_index_; i < zero_index_ + size_; ++i)
      os << ' ' << buffer_[i];
    os << " <" << (capacity_ - size_ - zero_index_) << ">}";
  }
#endif
};

template<typename T, size_t initial_size>
constexpr DEVector<T, initial_size>::DEVector(DEVector const& orig) :
  buffer_{allocate_buffer(capacity_)},
  capacity_{orig.capacity_},
  size_{orig.size_},
  zero_index_{orig.zero_index_}
{
  std::copy(orig.begin(), orig.end(), begin());
}

template<typename T, size_t initial_size>
constexpr T* DEVector<T, initial_size>::allocate_buffer(size_t min_capacity)
{
  // Add +1 to min_capacity and then subtract sizeof(T) to account for a possible overhead of std::aligned_alloc.
  size_t bytes_to_allocate = utils::malloc_size((min_capacity + 1) * sizeof(T)) - sizeof(T);
  capacity_ = bytes_to_allocate / sizeof(T);
  return static_cast<T*>(std::aligned_alloc(alignof(T), bytes_to_allocate));
}

template<typename T, size_t initial_size>
constexpr void DEVector<T, initial_size>::reallocate(size_t front_cap, size_t back_cap)
{
  // Make sure there is at least front_cap front capacity and
  // back_cap back capacity.

  if (!buffer_)
  {
    buffer_ = allocate_buffer(std::max(initial_size, front_cap + back_cap + size_));
    zero_index_ = capacity_ / 2;
    return;
  }

  // Upon reallocation, the amount of allocated memory is doubled.
  //
  // If we have this buffer:

  //   0 1 2
  // [ A B C ]
  //   ^
  //   `-- zero_index_ = 0
  //       size_ = 3
  //       capacity_ = 3
  //
  // and we need to push something at the back then the capacity is doubled
  // and the elements are put in the middle (rounded down):
  //
  //   0 1 2 3 4 5
  // [ - A B C - - ]
  //     ^
  //     `-- zero_index_ = 1
  //         size_ = 3
  //         capacity_ = 6
  //
  // Therefore, if the situation is like follows:
  //
  //   0 1 2 3 4 5 6 7 8 9 10
  // [ - - - - - - - - A B C ]
  //                   ^
  //                   `-- zero_index_ = 8
  //                       size_ = 3
  //                       capacity_ = 11
  //
  // we don't want to reallocate, but will simply move the elements
  // in the existing buffer to the middle:
  //
  //   0 1 2 3 4 5 6 7 8 9 10
  // [ - - - - A B C - - - - ]
  //           ^
  //           `-- zero_index_ = 4
  //               size_ = 3
  //               capacity_ = 11
  //
  // Therefore we will not reallocate if the size_ (after adding the requested front/back capacity)
  // is less than or equal half of the current capacity_.
  if (2 * (front_cap + size_ + back_cap) <= capacity_)
  {
    size_t new_zero_index = (capacity_ - size_ - back_cap) / 2;
    if (new_zero_index < zero_index_)
      std::copy(begin(), end(), buffer_ + new_zero_index);
    else
      std::copy_backward(begin(), end(), buffer_ + new_zero_index + size_);
    zero_index_ = new_zero_index;
  }
  else
  {
    // Increase the capacity to double what is needed.
    T* new_buffer = allocate_buffer(2 * (front_cap + size_ + back_cap));
    size_t new_zero_index = (capacity_ - size_ - back_cap) / 2;
    std::copy(begin(), end(), new_buffer + new_zero_index);
    zero_index_ = new_zero_index;
    std::free(buffer_);
    buffer_ = new_buffer;
  }
  ASSERT(zero_index_ >= front_cap && capacity_ - (zero_index_ - 1) - size_ >= back_cap);
}

template<typename T, size_t initial_size>
template<typename... Args>
constexpr typename DEVector<T, initial_size>::iterator DEVector<T, initial_size>::emplace(const_iterator pos, Args&&... args)
{
  // Index into buffer_ where the new element must be inserted.
  int i = pos - buffer_;
  //   0 1 2 3 4 5 6
  //  [- - A B C - - ]
  //       ^ ^
  //       | `-- pos --> i = 3
  //       `-- zero_index_ = 2
  //           size_ = 3
  //           capacity_ = 7
  //
  // After insertion of E at pos we will end up with:
  //
  //   0 1 2 3 4 5 6
  //  [- A E B C - - ]
  //
  // so that we only had to move A.
  //
  // If we want to move the left part, because that is smaller than the right part,
  // and there is room to to do that on the left...
  if (i < zero_index_ + size_ / 2 && zero_index_ >= 1)
  {
    // Move all elements before pos one position to the left.
    const_iterator begin_ = begin();
    std::copy(begin_, pos, begin() - 1);
    --zero_index_;
    --i;
  }
  else if (capacity_ - zero_index_ - size_ >= 1)      // Is there room on the right?
  {
    // Move the elements at pos and beyond one position to the right.
    const_iterator end_ = end();
    std::copy_backward(pos, end_, end() + 1);
  }
  else
  {
    size_t const number_of_elements_left = i - zero_index_;
    size_t const number_of_elements_right = size_ - number_of_elements_left;
    T* new_buffer = allocate_buffer(2 * (size_ + 1));
    size_t new_zero_index = (capacity_ - size_ - 1) / 2;
    i = new_zero_index + number_of_elements_left;
    std::copy(begin(), begin() + number_of_elements_left, new_buffer + new_zero_index);
    std::copy(end() - number_of_elements_right, end(), new_buffer + i + 1);
    std::free(buffer_);
    buffer_ = new_buffer;
    zero_index_ = new_zero_index;
  }

  ++size_;
  return new (buffer_ + i) T(std::forward<Args>(args)...);
}

} // namespace utils
