#pragma once

#include "Badge.h"
#include <concepts>

namespace utils {

template<std::integral T>
class UniqueIDContext;

template<std::integral T>
class UniqueID
{
 private:
  T const m_id;

 public:
  UniqueID(Badge<UniqueIDContext<T>>, T id) : m_id(id) { }

  operator T() const { return m_id; }
  bool operator==(UniqueID other) const { return m_id == other.m_id; }
  bool operator<(UniqueID other) const { return m_id < other.m_id; }
};

template<std::integral T>
class UniqueIDContext
{
 private:
  std::atomic<T> m_next_id;

 public:
  UniqueIDContext() = default;

  UniqueID<T> get_id()
  {
    return { {}, m_next_id++ };
  }
};

} // namespace utils
