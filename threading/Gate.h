// SPDX-FileCopyrightText: 2021-2022, 2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of class Gate.
 */

#pragma once

#include <condition_variable>

namespace utils::threading {

// Block (multiple) thread(s) until open() is called.
//
// If open() was already called before wait() then
// wait() also doesn't block anymore.
//
class Gate : public std::mutex
{
 private:
  std::condition_variable m_condition_variable;
  bool m_open;

 public:
  Gate() : m_open(false) { }

  void wait()
  {
    std::unique_lock<std::mutex> lk(*this);
    m_condition_variable.wait(lk, [this](){ return m_open; });
  }

  void open()
  {
    {
      std::lock_guard<std::mutex> lk(*this);
      m_open = true;
    }
    m_condition_variable.notify_all();
  }
};

} // namespace utils::threading
