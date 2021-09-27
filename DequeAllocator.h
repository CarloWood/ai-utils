#pragma once

#include "DequeMemoryResource.h"
#include <type_traits>

namespace utils {

// This allocator is intended to be used with std::deque.
//
// It uses utils::NodeMemoryResource for the fixed size
// allocations of it's elements, and DequeMemoryResource
// for the exponentially growing sizes of the internal
// table that a std::deque allocates; which in turn uses
// 12 NodeMemoryResource's for the smaller sizes, and
// malloc above that.
//
// Usage:
//
// At the top of main():
//
//   utils::MemoryPagePool mpp(0x8000);                                 // Allocate 32kB at a time (or larger powers of two are fine too).
//   utils::DequeMemoryResource::Initialization dmrs(&mpp);
//
// Creating the allocator:
//
//   utils::NodeMemoryResource nmr(mpp);                                // For the fixed size element allocation.
//   utils::DequeAllocator<Foo> allocator(nmr);
//
// Or, if you're using AIStatefulTask:
//
// At the top of main():
//
//   AIMemoryPagePool mpp;
//   utils::DequeMemoryResource::Initialization dmri(mpp.instance());
//
// Creating the allocator:
//
//   utils::NodeMemoryResource nmr(AIMemoryPagePool::instance());      // For the fixed size element allocation.
//   utils::DequeAllocator<Foo> allocator(nmr);
//
// And then create std::deque's with the allocator.
//
//   std::deque<Foo, decltype(allocator)> d1([...,] allocator);
//   std::deque<Foo, decltype(allocator)> d2([...,] allocator);
//   ...
//
// The same nmr can be used for multiple DequeAllocator's if the
// size of their type is equal, or if you add an element to the
// deque with the largest element first, or when you pass the
// size of the largest element to the nmr upon construction:
//
//   struct Foo37 { char x[37]; };
//   struct Foo38 { char x[38]; };
//
//   utils::NodeMemoryResource nmr(mpp, sizeof(Foo38));
//   utils::DequeAllocator<Foo37> allocator37(nmr);
//   utils::DequeAllocator<Foo38> allocator38(nmr);
//   etc.
//
template<typename T, typename ElementType = T>
class DequeAllocator
{
 private:
  utils::NodeMemoryResource* m_node_memory_resource;

 public:
  using value_type = T;

  // m_node_memory_resource is not used when T = ElementType*.
  using is_always_equal = std::conditional_t<std::is_same_v<T, ElementType>, std::false_type, std::true_type>;

  DequeAllocator(utils::NodeMemoryResource& node_memory_resource) : m_node_memory_resource(&node_memory_resource) { }

  using propagate_on_container_copy_assignment = std::true_type;
  DequeAllocator(DequeAllocator const& allocator) noexcept : m_node_memory_resource(allocator.m_node_memory_resource) { }

  using propagate_on_container_move_assignment = std::true_type;
  DequeAllocator& operator=(DequeAllocator const& allocator) noexcept
  {
    m_node_memory_resource = allocator.m_node_memory_resource;
    return *this;
  }

  using propagate_on_container_swap = std::true_type;
  void swap(DequeAllocator& other) { std::swap(m_node_memory_resource, other.m_node_memory_resource); }

  // Used by the constructor below.
  utils::NodeMemoryResource* nmr_ptr() const { return m_node_memory_resource; }

  // m_node_memory_resource is copied, but not expected to be used!
  // The only reason it is copied is because the standard requires that if the original type is constructed
  // from this result, it has to compare equal.
  template<typename U>
  DequeAllocator(DequeAllocator<U, ElementType> const& other) : m_node_memory_resource(other.nmr_ptr()) { }

  [[nodiscard]] T* allocate(std::size_t number_of_objects);
  void deallocate(T* p, std::size_t n) noexcept;

  friend bool operator==(DequeAllocator<T, ElementType> const& a1, DequeAllocator<T, ElementType> const& a2) noexcept
  {
    if constexpr (std::is_same_v<T, ElementType>)
      return a1.m_node_memory_resource == a2.m_node_memory_resource;
    else
      return true;
  }

  friend bool operator!=(DequeAllocator<T, ElementType> const& a1, DequeAllocator<T, ElementType> const& a2) noexcept
  {
    return !(a1 == a2);
  }

  DequeAllocator select_on_container_copy_construction() { return {*m_node_memory_resource}; }
};

template<typename T, typename ElementType>
T* DequeAllocator<T, ElementType>::allocate(std::size_t number_of_objects)
{
  T* ptr;
  if constexpr (std::is_same_v<T, ElementType>)
    ptr = static_cast<T*>(m_node_memory_resource->allocate(number_of_objects * sizeof(T)));
  else
    ptr = static_cast<T*>(DequeMemoryResource::s_instance.allocate(number_of_objects * sizeof(T)));
  return ptr;
}

template<typename T, typename ElementType>
void DequeAllocator<T, ElementType>::deallocate(T* p, std::size_t number_of_objects) noexcept
{
  if constexpr (std::is_same_v<T, ElementType>)
    m_node_memory_resource->deallocate(p);
  else
    DequeMemoryResource::s_instance.deallocate(p, number_of_objects * sizeof(T));
}

} // namespace utils
