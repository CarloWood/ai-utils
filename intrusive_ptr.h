#pragma once

#include <iosfwd>

namespace utils {

//
// intrusive_ptr
//
// A smart pointer that uses intrusive reference counting.
//
// Relies on unqualified calls to
//
//     void intrusive_ptr_add_ref(T* p);
//     void intrusive_ptr_release(T* p);
//
//         (p != 0)
//
// The object is responsible for destroying itself.
//

template<class T>
class intrusive_ptr
{
 private:
  T* m_px;

  using this_type = intrusive_ptr;

 public:
  using element_type = T;

  constexpr intrusive_ptr() : m_px(nullptr) { }

  intrusive_ptr(T* p, bool add_ref = true) : m_px(p)
  {
    if (m_px != nullptr && add_ref)
      intrusive_ptr_add_ref(m_px);
  }

  template<class U>
  intrusive_ptr(intrusive_ptr<U> const& rhs, std::enable_if_t<std::is_convertible_v<U, T>, int> = 0) : m_px(rhs.get())
  {
    if (m_px != nullptr)
      intrusive_ptr_add_ref(m_px);
  }

  intrusive_ptr(intrusive_ptr const& rhs) : m_px(rhs.m_px)
  {
    if (m_px != 0)
      intrusive_ptr_add_ref(m_px);
  }

  ~intrusive_ptr()
  {
    if (m_px != nullptr)
      intrusive_ptr_release(m_px);
  }

  template<class U>
  intrusive_ptr& operator=(intrusive_ptr<U> const& rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }

  // Move support
  intrusive_ptr(intrusive_ptr&& rhs) : m_px(rhs.m_px) { rhs.m_px = 0; }

  intrusive_ptr& operator=(intrusive_ptr&& rhs)
  {
    this_type(static_cast<intrusive_ptr&&>(rhs)).swap(*this);
    return *this;
  }

  template<class U>
  friend class intrusive_ptr;

  template<class U>
  intrusive_ptr(intrusive_ptr<U>&& rhs, std::enable_if_t<std::is_convertible_v<U, T>, int> = 0) : m_px(rhs.m_px)
  {
    rhs.m_px = nullptr;
  }

  template<class U>
  intrusive_ptr& operator=(intrusive_ptr<U>&& rhs)
  {
    this_type(static_cast<intrusive_ptr<U>&&>(rhs)).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(intrusive_ptr const& rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }

  intrusive_ptr& operator=(T* rhs)
  {
    this_type(rhs).swap(*this);
    return *this;
  }

  void reset() { this_type().swap(*this); }

  void reset(T* rhs) { this_type(rhs).swap(*this); }

  void reset(T* rhs, bool add_ref) { this_type(rhs, add_ref).swap(*this); }

  T* get() const { return m_px; }

  T* detach()
  {
    T* ret = m_px;
    m_px = nullptr;
    return ret;
  }

  T& operator*() const
  {
    ASSERT(m_px != nullptr);
    return *m_px;
  }

  T* operator->() const
  {
    ASSERT(m_px != nullptr);
    return m_px;
  }

  explicit operator bool () const
  {
    return m_px != nullptr;
  }

  void swap(intrusive_ptr& rhs)
  {
    T* tmp = m_px;
    m_px = rhs.m_px;
    rhs.m_px = tmp;
  }
};

template<class T, class U>
inline bool operator==(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b)
{
  return a.get() == b.get();
}

template<class T, class U>
inline bool operator!=(intrusive_ptr<T> const& a, intrusive_ptr<U> const& b)
{
  return a.get() != b.get();
}

template<class T, class U>
inline bool operator==(intrusive_ptr<T> const& a, U* b)
{
  return a.get() == b;
}

template<class T, class U>
inline bool operator!=(intrusive_ptr<T> const& a, U* b)
{
  return a.get() != b;
}

template<class T, class U>
inline bool operator==(T* a, intrusive_ptr<U> const& b)
{
  return a == b.get();
}

template<class T, class U>
inline bool operator!=(T* a, intrusive_ptr<U> const& b)
{
  return a != b.get();
}

template<class T>
inline bool operator==(intrusive_ptr<T> const& p, std::nullptr_t)
{
  return p.get() == 0;
}

template<class T>
inline bool operator==(std::nullptr_t, intrusive_ptr<T> const& p)
{
  return p.get() == 0;
}

template<class T>
inline bool operator!=(intrusive_ptr<T> const& p, std::nullptr_t)
{
  return p.get() != 0;
}

template<class T>
inline bool operator!=(std::nullptr_t, intrusive_ptr<T> const& p)
{
  return p.get() != 0;
}

template<class T>
inline bool operator<(intrusive_ptr<T> const& a, intrusive_ptr<T> const& b)
{
  return std::less<T*>()(a.get(), b.get());
}

template<class T>
void swap(intrusive_ptr<T>& lhs, intrusive_ptr<T>& rhs)
{
  lhs.swap(rhs);
}

// mem_fn support

template<class T>
T* get_pointer(intrusive_ptr<T> const& p)
{
  return p.get();
}

// pointer casts

template<class T, class U>
intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U> const& p)
{
  return static_cast<T*>(p.get());
}

template<class T, class U>
intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U> const& p)
{
  return const_cast<T*>(p.get());
}

template<class T, class U>
intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U> const& p)
{
  return dynamic_cast<T*>(p.get());
}

template<class T, class U>
intrusive_ptr<T> static_pointer_cast(intrusive_ptr<U>&& p)
{
  return intrusive_ptr<T>(static_cast<T*>(p.detach()), false);
}

template<class T, class U>
intrusive_ptr<T> const_pointer_cast(intrusive_ptr<U>&& p)
{
  return intrusive_ptr<T>(const_cast<T*>(p.detach()), false);
}

template<class T, class U>
intrusive_ptr<T> dynamic_pointer_cast(intrusive_ptr<U>&& p)
{
  T* p2 = dynamic_cast<T*>(p.get());

  intrusive_ptr<T> r(p2, false);

  if (p2)
    p.detach();

  return r;
}

// operator<<

template<typename T>
std::ostream& operator<<(std::ostream& os, intrusive_ptr<T> const& p)
{
  os << p.get();
  return os;
}

} // namespace utils

namespace std {

template<class T>
struct hash<::utils::intrusive_ptr<T>>
{
  std::size_t operator()(::utils::intrusive_ptr<T> const& p) const
  {
    return std::hash<T*>()(p.get());
  }
};

} // namespace std
