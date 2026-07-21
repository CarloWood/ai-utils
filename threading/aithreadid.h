// SPDX-FileCopyrightText: 2015, 2021 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * threadsafe -- Threading utilities: object oriented (read/write) locking and more.
 *
 * @file
 * @brief Declarations related to std::thread::id.
 *
 * CHANGELOG
 *   and additional copyright holders.
 *
 *   2015/02/27
 *   - Initial version, written by Aleric Inglewood @ SL
 *
 *   2016/12/17
 *   - Transfered copyright to Carlo Wood.
 *
 *   2026/07/21
 *   - Changed license to MIT.
 */

#pragma once

#include "utils/macros.h"
#include <thread>

namespace aithreadid
{

extern std::thread::id const none;
extern std::thread::id const s_main_thread_id;

// Debugging function.
// Usage:
//
//   static std::thread::id s_id;
//   assert(aithreadid::is_single_threaded(s_id));	// Fails if more than one thread executes this line.
inline bool is_single_threaded(std::thread::id& thread_id)
{
  if (AI_LIKELY(thread_id == std::this_thread::get_id()))
    return true;
  bool const first_call = thread_id == none;
  if (AI_LIKELY(first_call))
    thread_id = std::this_thread::get_id();
  return first_call;
}

// Debugging function.
// Usage:
//
//   assert(in_main_thread());                          // Fails if this is not the main thread. Only use after main() was reached.
inline bool in_main_thread()
{
  return s_main_thread_id == std::this_thread::get_id();
}

} // namespace aithreadid
