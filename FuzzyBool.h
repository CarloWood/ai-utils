// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of FuzzyBool.
//
// Copyright (C) 2019 Carlo Wood.
//
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file is part of ai-utils.
//
// ai-utils is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ai-utils is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include "debug.h"

namespace utils {

enum FuzzyBoolEnum {
  fuzzy_true = 0,
  fuzzy_was_true = 1,
  fuzzy_was_false = 2,
  fuzzy_false = 3
};

struct FuzzyBoolPOD
{
  FuzzyBoolEnum m_val;
};

} // namespace utils

namespace fuzzy {

static constexpr utils::FuzzyBoolPOD True = { utils::fuzzy_true };
static constexpr utils::FuzzyBoolPOD WasTrue = { utils::fuzzy_was_true };
static constexpr utils::FuzzyBoolPOD WasFalse = { utils::fuzzy_was_false };
static constexpr utils::FuzzyBoolPOD False = { utils::fuzzy_false };

} // namespace fuzzy

namespace utils {

class FuzzyBool : private FuzzyBoolPOD
{
 private:
  FuzzyBool(FuzzyBoolEnum val) { m_val = val; }

 public:
  // Uninitialized.
  FuzzyBool() { }
  // Contruct from a literal FuzzyBool (True/WasTrue/WasFalse/False) (also serves as copy constructor).
  FuzzyBool(FuzzyBoolPOD val) { m_val = val.m_val; }
  // Assignment.
  FuzzyBool& operator=(FuzzyBool const& val) { m_val = val.m_val; return *this; }
  // Construct from a non-literal bool (use True / False for literals).
  explicit FuzzyBool(bool val) { m_val = val ? fuzzy_true : fuzzy_false; }
  // Printing.
  void print_on(std::ostream& os);
  friend std::ostream& operator<<(std::ostream& os, FuzzyBool fb) { fb.print_on(os); return os; }
  // Accessors.
  bool always() const { return m_val == fuzzy_true; }
  bool likely() const { return m_val <= 1; }
  bool unlikely() const { return m_val >= 2; }
  bool never() const { return m_val == fuzzy_false; }
#if CWDEBUG
  bool has_same_value_as(FuzzyBool fb) { return m_val == fb.m_val; }
#endif
  // Only use this when the value is certain.
  operator bool() const { ASSERT(m_val == fuzzy_true || m_val == fuzzy_false); return m_val == fuzzy_true; }
  // Logic.
  //
  // Operator NOT; !True = False, !!x = x, !x != x.
  // fb1 | !fb1
  //  0  |  3
  //  1  |  2
  //  2  |  1
  //  3  |  0
  //
  FuzzyBool operator!() { return static_cast<FuzzyBoolEnum>(3 - m_val); }
  //
  // Operator AND; True && x = x, False && x = False, x && x = x, x && y = y && x, wasTrue && wasFalse = wasFalse.
  // fb1 | fb2: 0  1  2  3
  // ---------------------
  //  0  |      0  1  2  3
  //  1  |      1  1  2  3
  //  2  |      2  2  2  3
  //  3  |      3  3  3  3
  //
  friend FuzzyBool operator&&(FuzzyBool fb1, FuzzyBool fb2) { return static_cast<FuzzyBoolEnum>(std::max(fb1.m_val, fb2.m_val)); }
  //
  // Operator OR, x || y = !(!x && !y)
  // fb1 | fb2: 0  1  2  3
  // ---------------------
  //  0  |      0  0  0  0
  //  1  |      0  1  1  1
  //  2  |      0  1  2  2
  //  3  |      0  1  2  3
  //
  friend FuzzyBool operator||(FuzzyBool fb1, FuzzyBool fb2) { return static_cast<FuzzyBoolEnum>(std::min(fb1.m_val, fb2.m_val)); }
  //
  // Operator XOR, x ^ y = (x && !y) || (!x && y)
  // fb1 | fb2: 0  1  2  3
  // ---------------------
  //  0  |      3  2  1  0
  //  1  |      2  2  1  1
  //  2  |      1  1  2  2
  //  3  |      0  1  2  3
  //
  friend FuzzyBool operator!=(FuzzyBool fb1, FuzzyBool fb2) { return static_cast<FuzzyBoolEnum>(std::min(std::max(3 - fb1.m_val, 0 + fb2.m_val), std::max(0 + fb1.m_val, 3 - fb2.m_val))); }
  //
  // Operator NOT XOR
  // fb1 | fb2: 0  1  2  3
  // --------------------------------------
  //  0  |      0  1  2  3
  //  1  |      1  1  2  2
  //  2  |      2  2  1  1
  //  3  |      3  2  1  0
  //
  friend FuzzyBool operator==(FuzzyBool fb1, FuzzyBool fb2) { return static_cast<FuzzyBoolEnum>(std::max(std::min(3 - fb1.m_val, 0 + fb2.m_val), std::min(0 + fb1.m_val, 3 - fb2.m_val))); }
};

} // namespace utils
