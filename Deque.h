#include "VectorIndex.h"
#include <deque>

namespace utils {

template <typename T, typename _Index = VectorIndex<T>, typename _Alloc = std::allocator<T>>
class Deque : public std::deque<T, _Alloc>
{
 protected:
  using _Base = std::deque<T, _Alloc>;

 public:
  using reference = typename _Base::reference;
  using const_reference = typename _Base::const_reference;
  using index_type = _Index;

  using std::deque<T, _Alloc>::deque;
  using std::deque<T, _Alloc>::operator=;

 public:
  reference operator[](index_type __n) _GLIBCXX_NOEXCEPT { return _Base::operator[](static_cast<size_t>(__n)); }
  const_reference operator[](index_type __n) const _GLIBCXX_NOEXCEPT { return _Base::operator[](static_cast<size_t>(__n)); }

  reference at(index_type __n) { return _Base::at(static_cast<size_t>(__n)); }
  const_reference at(index_type __n) const { return _Base::at(static_cast<size_t>(__n)); }

  index_type ibegin() const { return index_type(size_t{0}); }
  index_type iend() const { return index_type(_Base::size()); }
};

} // namespace utils
