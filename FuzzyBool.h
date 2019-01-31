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

#define F 0
#define U 4
#define L 8
#define T c

enum FuzzyBoolEnum {
  fuzzy_false           = F,        // F: False
  fuzzy_was_false       = U,        // U: Unlikely
  fuzzy_was_true        = L,        // L: Likely
  fuzzy_true            = 0xc       // T: True
};

struct FuzzyBoolPOD
{
  FuzzyBoolEnum m_val;
};

extern void print_FuzzyBoolPOD_on(std::ostream& os, FuzzyBoolEnum val);
inline std::ostream& operator<<(std::ostream& os, FuzzyBoolPOD const& fb) { print_FuzzyBoolPOD_on(os, fb.m_val); return os; }

} // namespace utils

namespace fuzzy {

static constexpr utils::FuzzyBoolPOD True = { utils::fuzzy_true };              // If you read the value True, it is really true.
static constexpr utils::FuzzyBoolPOD WasTrue = { utils::fuzzy_was_true };       // If you read the value WasTrue, then very recently it was true, but maybe not anymore.
static constexpr utils::FuzzyBoolPOD WasFalse = { utils::fuzzy_was_false };     // If you read the value WasFalse, then very recently it was false, but maybe not anymore.
static constexpr utils::FuzzyBoolPOD False = { utils::fuzzy_false };            // If you read the value False, it is really false.

} // namespace fuzzy

namespace utils {

class FuzzyBool : public FuzzyBoolPOD
{
  // For internal usage (creating return values). Construct a FuzzyBool from an FuzzyBoolEnum.
  FuzzyBool(FuzzyBoolEnum val) : FuzzyBoolPOD{val} { }

 public:
  // Construct an uninitialized FuzzyBool.
  FuzzyBool() { }

  // Copy-constructor.
  FuzzyBool(FuzzyBoolPOD const& val) : FuzzyBoolPOD{val.m_val} { }

  // Assignment.
  FuzzyBool& operator=(FuzzyBoolPOD const& val) { m_val = val.m_val; return *this; }

  // Construct from a non-literal bool (use True / False for literals).
  explicit constexpr FuzzyBool(bool val) : FuzzyBoolPOD{val ? fuzzy_true : fuzzy_false} { }

  // Printing.
  void print_on(std::ostream& os) const { print_FuzzyBoolPOD_on(os, m_val); }

  // Accessors.
  bool always() const { return m_val == fuzzy_true; }   // Returns true when True.
  bool likely() const { return m_val & L; }             // Returns true when WasTrue or True.
  bool unlikely() const { return !(m_val & L); }        // Returns true when WasFalse or False.
  bool never() const { return m_val == fuzzy_false; }   // Returns true when False.
#ifdef CWDEBUG
  bool has_same_value_as(FuzzyBool const& fb) { return m_val == fb.m_val; }
#endif
  // Only use this when the value is certain.
  explicit operator bool() const { ASSERT(m_val == fuzzy_true || m_val == fuzzy_false); return m_val == fuzzy_true; }

  // Logic.

  // Operator NOT; !T = F, !!x = x, !x != x.
  // fb1 | !fb1
  //  F  |  T
  //  U  |  L
  //  L  |  U
  //  T  |  F
  //
  FuzzyBool operator!() const noexcept { return static_cast<FuzzyBoolEnum>(m_val ^ 0xc); }

  [[gnu::always_inline]] friend inline FuzzyBool operator&&(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept;
  [[gnu::always_inline]] friend inline FuzzyBool operator||(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept;
  [[gnu::always_inline]] friend inline FuzzyBool operator!=(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept;
  [[gnu::always_inline]] friend inline FuzzyBool operator==(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept;
};

namespace {

#define TABLE2(e00, e10, e20, e30, \
               e01, e11, e21, e31, \
               e02, e12, e22, e32, \
               e03, e13, e23, e33) \
  { 0x##e33##e32##e31##e30##e23##e22##e21##e20##e13##e12##e11##e10##e03##e02##e01##e00 }

#define TABLE(e00, e10, e20, e30, e01, e11, e21, e31, e02, e12, e22, e32, e03, e13, e23, e33) \
       TABLE2(e00, e10, e20, e30, e01, e11, e21, e31, e02, e12, e22, e32, e03, e13, e23, e33)

static constexpr uint64_t AND_table = TABLE(
// Operator AND; True && x = x, False && x = False, x && x = x, x && y = y && x, wasTrue && wasFalse = wasFalse.
// fb1 | fb2: F  U  L  T
//     ------------------
  /* F | */   F, F, F, F,
  /* U | */   F, U, U, U,
  /* L | */   F, U, L, L,
  /* T | */   F, U, L, T
);

static constexpr uint64_t OR_table = TABLE(
// Operator OR, x || y = !(!x && !y)
// fb1 | fb2: F  U  L  T
//     ------------------
  /* F | */   F, U, L, T,
  /* U | */   U, U, L, T,
  /* L | */   L, L, L, T,
  /* T | */   T, T, T, T
);

static constexpr uint64_t XOR_table = TABLE(
// Operator XOR, x ^ y = (x && !y) || (!x && y)
// fb1 | fb2: F  U  L  T
//     ------------------
  /* F | */   F, U, L, T,
  /* U | */   U, U, L, L,
  /* L | */   L, L, U, U,
  /* T | */   T, L, U, F
);

static constexpr uint64_t NOT_XOR_table = TABLE(
// Operator NOT XOR
// fb1 | fb2: F  U  L  T
//     ------------------
  /* F | */   T, L, U, F,
  /* U | */   L, L, U, U,
  /* L | */   U, U, L, L,
  /* T | */   F, U, L, T
);

} // namespace

FuzzyBool operator&&(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept
{
  return static_cast<FuzzyBoolEnum>((AND_table >> (4 * fb1.m_val + fb2.m_val)) & 0xc);
}

FuzzyBool operator||(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept
{
  return static_cast<FuzzyBoolEnum>((OR_table >> (4 * fb1.m_val + fb2.m_val)) & 0xc);
}

FuzzyBool operator!=(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept
{
  return static_cast<FuzzyBoolEnum>((XOR_table >> (4 * fb1.m_val + fb2.m_val)) & 0xc);
}

FuzzyBool operator==(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept
{
  return static_cast<FuzzyBoolEnum>((NOT_XOR_table >> (4 * fb1.m_val + fb2.m_val)) & 0xf);
}

#undef F
#undef U
#undef L
#undef T
#undef TABLE
#undef TABLE2

} // namespace utils
