// SPDX-FileCopyrightText: 2019, 2021 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of unlock_guard.
 */

#pragma once

namespace utils::threading {

template<typename T>
class unlock_guard
{
 private:
  T& m_mutex;

 public:
  unlock_guard(T& mutex) : m_mutex(mutex) { m_mutex.unlock(); }
  unlock_guard(unlock_guard const&) = delete;
  unlock_guard(unlock_guard&&) = delete;
  ~unlock_guard() { m_mutex.lock(); }
};

} // namespace utils::threading
