/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of class StartingGate.
 *
 * @Copyright (C) 2019  Carlo Wood.
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

#include "Gate.h"

namespace utils::threading
{

// Block threads until `stalls' threads are ready.
//
class StartingGate
{
 private:
  std::atomic_int m_stalls;
  Gate m_gate;

 public:
  StartingGate(int stalls) : m_stalls(stalls) { }

  void wait()
  {
    if (m_stalls-- == 1)
      m_gate.open();
    m_gate.wait();
  }
};

} // namespace utils::threading
