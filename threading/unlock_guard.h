/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of unlock_guard.
 *
 * @Copyright (C) 2021  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
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
