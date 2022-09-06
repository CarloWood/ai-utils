#pragma once

#include "Badge.h"
#include <atomic>
#include <concepts>

namespace utils {

template<typename T>
class UniqueIDContext;

template<typename Q>
concept SaneUniqueIDT = sizeof(Q) <= 2 * sizeof(long);

template<typename Q>
concept LargeUniqueIDT = sizeof(Q) > 2 * sizeof(long);

template<typename T>
class UniqueID
{
 private:
  T m_id;

 public:
  UniqueID(Badge<UniqueIDContext<T>>, T id) : m_id(std::move(id)) { }

  // By value - the size of T is expected to be small under normal circumstances.
  template<SaneUniqueIDT Q = T> operator T() const { return m_id; }
  template<SaneUniqueIDT Q = T> friend bool operator==(UniqueID lhs, UniqueID rhs) { return lhs.m_id == rhs.m_id; }
  template<SaneUniqueIDT Q = T> friend bool operator<(UniqueID lhs, UniqueID rhs) { return lhs.m_id < rhs.m_id; }

  // And well...
  template<LargeUniqueIDT Q = T> operator T const&() const { return m_id; }
  template<LargeUniqueIDT Q = T> friend bool operator==(UniqueID const& lhs, UniqueID const& rhs) { return lhs.m_id == rhs.m_id; }
  template<LargeUniqueIDT Q = T> friend bool operator<(UniqueID const& lhs, UniqueID const& rhs) { return lhs.m_id < rhs.m_id; }
};

template<std::integral T>
class UniqueIDContext<T>
{
 private:
  static constexpr bool thread_safe = true;
  std::atomic<T> m_next_id;

 public:
  UniqueIDContext() = default;

  UniqueID<T> get_id()
  {
    return { {}, m_next_id++ };
  }
};

// Non-integral version is not thread-safe!
template<typename T> requires (!std::integral<T>)
class UniqueIDContext<T>
{
 private:
  static constexpr bool thread_safe = false;
  T m_next_id;

 public:
  UniqueIDContext(T initial_value) : m_next_id(std::move(initial_value)) { }

  UniqueID<T> get_id()
  {
    return { {}, m_next_id++ };
  }
};

} // namespace utils
