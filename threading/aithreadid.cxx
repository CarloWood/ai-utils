// SPDX-FileCopyrightText: 2015, 2021 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * threadsafe -- Threading utilities: object oriented (read/write) locking and more.
 *
 * @file
 * @brief Definition of the constant s_main_thread_id.
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

#include "sys.h"
#include "aithreadid.h"

namespace aithreadid
{

std::thread::id const s_main_thread_id = std::this_thread::get_id();
std::thread::id const none;

}
