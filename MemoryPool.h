#include <vector>

namespace utils {

struct FreeList;

class MemoryPool
{
 private:
  FreeList* m_free_list;
  std::vector<void*> m_blocks;

  void* alloc(size_t size);

 public:
  MemoryPool() : m_free_list(nullptr) { }

  template<class Tp>
  Tp* malloc() { return static_cast<Tp*>(alloc(sizeof(Tp))); }

  void free(void* ptr);
};

} // namespace utils
