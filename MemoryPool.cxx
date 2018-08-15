#include "sys.h"
#include "MemoryPool.h"
#include "macros.h"
#include <cstdlib>

namespace utils {

static constexpr size_t N = 1024;

struct FreeList;

union Next
{
  std::size_t n;                // This must have the same size as a pointer, so that setting this to zero causes ptr == nullptr.
  FreeList* ptr;
};

struct FreeList
{
  Next m_next;
};

void* MemoryPool::alloc(size_t size)    // size must be larger than zero and a multiple of sizeof(FreeList).
{
  FreeList* ptr = m_free_list;
  if (AI_UNLIKELY(!ptr))
  {
    ptr = m_free_list = static_cast<FreeList*>(std::malloc(N * size));
    ptr->m_next.n = N - 1;
  }
  if (AI_UNLIKELY(ptr->m_next.n < N && ptr->m_next.ptr))
  {
    size_t n = ptr->m_next.n;
    ptr->m_next.ptr = reinterpret_cast<FreeList*>(reinterpret_cast<char*>(ptr) + size);
    ptr->m_next.ptr->m_next.n = n - 1;
  }
  m_free_list = ptr->m_next.ptr;
  return ptr;
}

void MemoryPool::free(void*)
{
}

} // namespace utils
