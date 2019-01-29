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
#include <atomic>

class FuzzyLock;

namespace utils {

enum FuzzyBoolEnum {
  fuzzy_false = 0,
  fuzzy_was_false = 1,
  fuzzy_was_true = 2,
  fuzzy_true = 3
};

struct FuzzyBoolPOD
{
  FuzzyBoolEnum const m_literal;
};

static_assert(std::is_pod<FuzzyBoolPOD>::value, "FuzzyBoolPOD should be POD :/");

extern void print_FuzzyBoolPOD_on(std::ostream& os, FuzzyBoolEnum val);
inline std::ostream& operator<<(std::ostream& os, FuzzyBoolPOD const& fb) { print_FuzzyBoolPOD_on(os, fb.m_literal); return os; }

} // namespace utils

namespace fuzzy {

static constexpr utils::FuzzyBoolPOD True = { utils::fuzzy_true };              // If you read the value True, it is really true.
static constexpr utils::FuzzyBoolPOD WasTrue = { utils::fuzzy_was_true };       // If you read the value WasTrue, then very recently it was true, but maybe not anymore.
static constexpr utils::FuzzyBoolPOD WasFalse = { utils::fuzzy_was_false };     // If you read the value WasFalse, then very recently it was false, but maybe not anymore.
static constexpr utils::FuzzyBoolPOD False = { utils::fuzzy_false };            // If you read the value False, it is really false.

} // namespace fuzzy

namespace utils {

class FuzzyBool
{
 private:
  // Like FuzzyBoolPOD, but atomic.
  //
  // Because we want to be able to do certain operations as atomic RMW operation there is the following requirement:
  // Let True have the bit-representation { tn, ...., t1, t0 }, False the bit-representation { fn, ..., f1, f0 },
  // WasTrue the bit-representation { ln, ..., l1, l0 } and WasFalse the bit-representation { un, ..., u1, u0 },
  // where n is the most significant bit and 0 the least significant bit.
  //
  // Ignoring the bits 2 and higher, we have
  //
  //              X       !X
  // True       t1 t0    f1 f0
  // WasTrue    l1 l0    u1 u0
  // WasFalse   u1 u0    l1 l0
  // False      f1 f0    t1 t0
  //
  // We only have the following RMW operations available: add, sub, and, or and xor.
  //
  std::atomic<FuzzyBoolEnum> m_val;

  // For internal usage (creating return values). Construct a FuzzyBool from an FuzzyBoolEnum.
  FuzzyBool(FuzzyBoolEnum val) : m_val{val} { }

 public:
  // Construct an uninitialized FuzzyBool.
  FuzzyBool() { }

  // Construct from a literal FuzzyBool (True/WasTrue/WasFalse/False).
  FuzzyBool(FuzzyBoolPOD val) : m_val{val.m_literal} { }

  // Cannot copy-construct from another atomic; however - using FuzzyLock *does* make this safe;
  // so require one to be passed.
  FuzzyBool(FuzzyBool const& val, FuzzyLock&)
  {
    // Assuming the FuzzyLock is correctly used (can we check that here?), we can make a copy by going through a temporary non-atomic.
    FuzzyBoolEnum tmp = val.m_val;
    m_val = tmp;
  }
  // Assignment.
  FuzzyBool& operator=(FuzzyBool const& val) = delete;  // Cannot assign one atomic to another.
  // But we *can* assign from a literal, of course.
  FuzzyBool& operator=(FuzzyBoolPOD val) { m_val = val.m_literal; return *this; }
  // Construct from a non-literal bool (use True / False for literals).
  explicit constexpr FuzzyBool(bool val) : m_val{val ? fuzzy_true : fuzzy_false} { }
  // Printing. Mostly using relaxed here because I don't want printing debug
  // output to interfere with the normal memory synchronization methods.
  // This is not really 100% thread-safe therefore, it's more a value of-the-moment.
  void print_on(std::ostream& os) const { print_FuzzyBoolPOD_on(os, m_val.load(std::memory_order_relaxed)); }
  friend std::ostream& operator<<(std::ostream& os, FuzzyBool const& fb) { fb.print_on(os); return os; }
  // Accessors.
  bool always() const { return m_val == fuzzy_true; }
  bool likely() const { return m_val >= 2; }
  bool unlikely() const { return m_val <= 1; }
  bool never() const { return m_val == fuzzy_false; }
#ifdef CWDEBUG
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
  //  0  |      0  0  0  0
  //  1  |      0  1  1  1
  //  2  |      0  1  2  2
  //  3  |      0  1  2  3
  //
  friend FuzzyBool operator&&(FuzzyBool fb1, FuzzyBool fb2) { return static_cast<FuzzyBoolEnum>(std::min(fb1.m_val, fb2.m_val)); }
  //
  // Operator OR, x || y = !(!x && !y)
  // fb1 | fb2: 0  1  2  3
  // ---------------------
  //  0  |      0  1  2  3
  //  1  |      1  1  2  3
  //  2  |      2  2  2  3
  //  3  |      3  3  3  3
  //
  friend FuzzyBool operator||(FuzzyBool fb1, FuzzyBool fb2) { return static_cast<FuzzyBoolEnum>(std::max(fb1.m_val, fb2.m_val)); }
  //
  // Operator XOR, x ^ y = (x && !y) || (!x && y)
  // fb1 | fb2: 0  1  2  3
  // ---------------------
  //  0  |      0  1  2  3
  //  1  |      1  1  2  2
  //  2  |      2  2  1  1
  //  3  |      3  2  1  0
  //
  friend FuzzyBool operator!=(FuzzyBool fb1, FuzzyBool fb2) { return static_cast<FuzzyBoolEnum>(std::max(std::min(3 - fb1.m_val, 0 + fb2.m_val), std::min(0 + fb1.m_val, 3 - fb2.m_val))); }
  //
  // Operator NOT XOR
  // fb1 | fb2: 0  1  2  3
  // --------------------------------------
  //  0  |      3  2  1  0
  //  1  |      2  2  1  1
  //  2  |      1  1  2  2
  //  3  |      0  1  2  3
  //
  friend FuzzyBool operator==(FuzzyBool fb1, FuzzyBool fb2) { return static_cast<FuzzyBoolEnum>(std::min(std::max(3 - fb1.m_val, 0 + fb2.m_val), std::max(0 + fb1.m_val, 3 - fb2.m_val))); }
};

} // namespace utils
