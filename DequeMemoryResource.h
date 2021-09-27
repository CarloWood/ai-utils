#pragma once

#include "utils/NodeMemoryResource.h"
#include "utils/log2.h"
#include <array>

namespace utils {

// A memory resource for the internal tables of std::deque.
// Only used by DequeAllocator, but the user must create a
// DequeMemoryResource::Initialization object at the top of main.
class DequeMemoryResource
{
 public:
  // std::deque<T, Allocator<T>> uses Allocator<T> to allocate fixed size
  // blocks of std::min(1, _GLIBCXX_DEQUE_BUF_SIZE / sizeof(T)) * sizeof(T) bytes,
  // but it uses Allocator<T*> to allocate exponentially growing tables of T*'s.
  // Where _GLIBCXX_DEQUE_BUF_SIZE is 512 by default.
  //
  // The minimum number of pointers (deque::_S_initial_map_size) is 8,
  // but the actual (initial) map size (_M_map_size) can be larger:
  // If a std::deque<_Tp> d(__num_elements); is created with __num_elements elements
  // then the initial _M_map_size is std::max(_S_initial_map_size, __num_nodes + 2),
  // where __num_nodes = __num_elements / __deque_buf_size(sizeof(_Tp)) + 1;
  // and __deque_buf_size(__size) is (__size < _GLIBCXX_DEQUE_BUF_SIZE) ? _GLIBCXX_DEQUE_BUF_SIZE / __size : 1;
  //
  // Aka, __deque_buf_size returns the number of elements that fit in a buffer
  // of size _GLIBCXX_DEQUE_BUF_SIZE, but at least 1. __num_nodes is then
  // the number of blocks with size max(_GLIBCXX_DEQUE_BUF_SIZE, sizeof(_Tp))
  // that are needed to store __num_elements objects of type _Tp, plus one.
  // And the initial map size begins larger than 8 when __num_nodes > 6.
  //
  // For simplicity lets assume that sizeof(_Tp) < 512, then the initial size
  // will be larger than 8 when we need more than 6 blocks of 512 bytes to store
  // __num_elements objects of _Tp, or roughly when __num_elements * sizeof(_Tp) > 3072.
  //
  // If subsequently the deque is grown "slowly" (that is, less elements are
  // added than what would roughly double the _M_map_size), we get a well-known
  // exponentially growing series of sizes for the map size. Namely, the _M_map_size
  // grows as: __new_map_size = _M_map_size + std::max(_M_map_size, __nodes_to_add) + 2;
  // So, if we never add more elements than what would result in __nodes_to_add
  // being larger than the existing _M_map_size, the growth can be predicted.
  // Aka, for the minimum map size of 8 (and sizeof(_Tp) < 512), less than
  // roughly 4096 / sizeof(_Tp).
  //
  // In conclusion by far the most frequently occuring _M_map_size's are
  // 8, 18, 38, 78, 158, 318, ... etc (multiply by 2 and add 2 each time).
  //
  // If we support these exact sizes we will normally be very (memory) efficient
  // for deque's. For random sizes we'd waste on average 25% of the allocated
  // memory however. We can make this a bit more efficient by supporting
  // intermediate sizes; that is - have an array of NodeMemoryResource's whose
  // index n corresponds to the block size:
  //
  //   s = (10 * 2^(n / 2) - 2) * sizeof(void*)
  //
  // The first block sizes that correspond with a given index then are:
  //
  //    n       s / sizeof(void*)
  //    0       8
  //    1       12
  //    2       18
  //    3       26
  //    4       38
  //    5       54
  //    6       78
  //    7       111
  //    8       158
  //    9       224
  //   10       318
  //   11       451
  //
  static constexpr int nmra_size = 12;  // NodeMemoryResource Array size.
  //
  // We need a function to convert those sizes to their index such that intermediate
  // sizes fall in the bucket with a size that is larger, of course: the ceil of
  // a function that gives exact values for the listed sizes, *especially* for
  // the most frequent occurring sizes of deques:
  //
  //    n = ceil(log2(( (s / sizeof(void*)) + 2 ) / 10.0))
  //
  // For the even n in the above table (8, 18, 38, 78, 158, ...) this
  // results in 0, 1, 2, 3, 4, ... while 9, 19, 39, 79, 159 gives
  // respectively 1, 2, 3, 4, 5. However, only if we calculate
  // the argument of the log2 as a floating point value, otherwise
  // we don't see an increment of 1 in s. This can be fixed
  // with the following trick:
  //
  //    n = ceil_log2(16 * ( (s / sizeof(void*)) + 2 ) / 10) - 4
  //
  // That is, multiplying with a power of 2 (16) that is larger
  // than 10 inside the log2 and subtracting the log2(16).
  //
  // Of course, this still has to be multiplied with 2 to get
  // the even indices:
  //
  //    n = 2 * ceil_log2(16 * ( (s / sizeof(void*)) + 2 ) / 10) - 8
  //
  // But then we'll never get odd indices.
  // To fix that, we can bring the 2* inside the log2 by squaring
  // its argument:
  //
  //    t = 16 * ( (s / sizeof(void*)) + 2 ) / 10;
  //    n = ceil_log2(t * t) - 8;
 private:
  static constexpr int size_to_index(std::size_t s)
  {
    // Should be the minimal map size, see std::deque<>::_S_initial_map_size.
    // If this asserts (in the future?) than theoretically we should set s to 8 * sizeof(void*).
    ASSERT(s >= 8 * sizeof(void*));
    // Do not call this function for sizes larger than 451 nodes.
    ASSERT(s <= 451 * sizeof(void*));
    unsigned int nodes = s / sizeof(void*);     // 8 <= nodes <= 451.
    unsigned int t = 16 * (nodes + 2) / 10;     // 16 <= t <= 724, 256 <= t^2 <= 524176.
    return utils::ceil_log2(t * t) - 8;         // 0 <= result <= 11.
  }

  // Used by the constructor of m_node_memory_resources.
  DequeMemoryResource() = default;

  // The actual initialization of m_node_memory_resources must be done after reaching main()
  // (after initialization of a MemoryPagePool).
  void init(MemoryPagePool* mpp_ptr);

 public:
  static DequeMemoryResource s_instance;

  // DequeMemoryResource must be initialized after creating the (a) MemoryPagePool, and
  // before using the first std::deque that uses utils::DequeAllocator.
  //
  // For example,
  //
  // utils::MemoryPagePool mpp(0x8000);                         // Allocate 32kB at a time.
  // utils::DequeMemoryResource::Initialization dmrs(&mpp);
  //
  struct Initialization
  {
    Initialization(MemoryPagePool& mpp_ptr)
    {
      s_instance.init(&mpp_ptr);
    }
  };

  void* allocate(std::size_t number_of_bytes);
  void deallocate(void* p, std::size_t number_of_bytes);

 private:
  // Use a NodeMemoryResource for the nmra_size smallest sizes.
  // The default memory pool allocates 32 kB blocks, so that means the largest value (451 * sizeof(void*) = 3608 bytes) needs a new call to malloc
  // every 9 allocations of 3608 bytes. Larger values (the next being 5104) are allocated directly with malloc.
  using node_memory_resources_container_t = std::array<NodeMemoryResource, nmra_size>;  // Note that the size of NodeMemoryResource is 64 bytes.
  node_memory_resources_container_t m_node_memory_resources = {};
};

} // namespace utils
