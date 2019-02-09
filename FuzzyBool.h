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

namespace {

// These values are and must be multiples of four (0, 1, 2 and 3 times 4).
static constexpr uint64_t F = 0;        // False
static constexpr uint64_t U = 4;        // Unlikely (WasFalse)
static constexpr uint64_t L = 8;        // Likely (WasTrue)
static constexpr uint64_t T = 12;       // True
static constexpr uint64_t bitmask = 12; // All bits used.

} // namespace

enum FuzzyBoolEnum : int {
  fuzzy_false           = F,
  fuzzy_was_false       = U,
  fuzzy_was_true        = L,
  fuzzy_true            = T
};

namespace {

constexpr uint64_t table(uint64_t e00, uint64_t e10, uint64_t e20, uint64_t e30,
                         uint64_t e01, uint64_t e11, uint64_t e21, uint64_t e31,
                         uint64_t e02, uint64_t e12, uint64_t e22, uint64_t e32,
                         uint64_t e03, uint64_t e13, uint64_t e23, uint64_t e33)
{
  return (e00      ) + (e10 << 16) + (e20 << 32) + (e30 << 48) +
         (e01 << 4 ) + (e11 << 20) + (e21 << 36) + (e31 << 52) +
         (e02 << 8 ) + (e12 << 24) + (e22 << 40) + (e32 << 56) +
         (e03 << 12) + (e13 << 28) + (e23 << 44) + (e33 << 60);
}

// Operator NOT
//
//    !T == F,
//   !!x == x,
//    !x != x.
//
// fb1 | !fb1
//  F  |  T
//  U  |  L
//  L  |  U
//  T  |  F
//

// Operator AND
//
//      True && x == x,
//     False && x == False,
//         x && x == x,
//         x && y == y && x,
//   wasTrue && WasFalse == WasFalse.
//
// The latter can be argued as follows:
// when a variable x has the value wasTrue then that means that it was
// just read from an atomic_bool that had the value true but that might have
// become false in the meantime, due to another thread writing false to it,
// in which case the correct value of x would be false (since it is
// supposed to be a copy of that atomic).
// Likewise when a variable y has the value WasFalse then that means
// that it was just read from an atomic_bool that had the value false but
// that might have become true in the meantime, due to another thread writing
// true to it, in which case the correct value of y would be true (since it
// is supposed to be a copy of that atomic).
//
// Hence the value x == y *was* true == false (which is false), but it
// might or might not have been changed (for example when x OR y changed
// value). Therefore it should be interpreted as WasFalse. Most notably
// it cannot be False, because means it is *certainly* false despite
// what other threads have done in the meantime, nor can it be True,
// nor can it be WasTrue -- because it wasn't.
//
// From the above rules the following table can be derived:
//
static constexpr uint64_t AND_table = table(
// fb1 | fb2: F  U  L  T
//     ------------------
  /* F | */   F, F, F, F,
  /* U | */   F, U, U, U,
  /* L | */   F, U, L, L,
  /* T | */   F, U, L, T
);

// Operator OR
//
//   x || y == !(!x && !y)
//
static constexpr uint64_t OR_table = table(
// fb1 | fb2: F  U  L  T
//     ------------------
  /* F | */   F, U, L, T,
  /* U | */   U, U, L, T,
  /* L | */   L, L, L, T,
  /* T | */   T, T, T, T
);

// Operator XOR
//
//   x ^ y = (x && !y) || (!x && y)
//
static constexpr uint64_t XOR_table = table(
// fb1 | fb2: F  U  L  T
//     ------------------
  /* F | */   F, U, L, T,
  /* U | */   U, U, L, L,
  /* L | */   L, L, U, U,
  /* T | */   T, L, U, F
);

// Operator NOT XOR
//
static constexpr uint64_t NOT_XOR_table = table(
// fb1 | fb2: F  U  L  T
//     ------------------
  /* F | */   T, L, U, F,
  /* U | */   L, L, U, U,
  /* L | */   U, U, L, L,
  /* T | */   F, U, L, T
);

} // namespace

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
 public:
  // Construct an uninitialized FuzzyBool.
  FuzzyBool() { }

  // Copy-constructor.
  FuzzyBool(FuzzyBoolPOD const& val) : FuzzyBoolPOD{val.m_val} { }

  // For internal usage (creating return values). Construct a FuzzyBool from an FuzzyBoolEnum.
  explicit constexpr FuzzyBool(FuzzyBoolEnum val) : FuzzyBoolPOD{val} { }

  // Construct from a non-literal bool (use True / False for literals).
  explicit constexpr FuzzyBool(bool val) : FuzzyBoolPOD{val ? fuzzy_true : fuzzy_false} { }

  // Assignment.
  FuzzyBool& operator=(FuzzyBoolPOD const& val) { m_val = val.m_val; return *this; }

  // Printing.
  void print_on(std::ostream& os) const { print_FuzzyBoolPOD_on(os, m_val); }

  // Accessors.
  bool is_true() const { return m_val == fuzzy_true; }                    // Returns true when True.
  bool is_transitory_true() const { return m_val == fuzzy_was_true; }     // Returns true when WasTrue.
  bool is_momentary_true() const { return m_val & L; }                    // Returns true when WasTrue or True.
  bool is_momentary_false() const { return !(m_val & L); }                // Returns true when WasFalse or False.
  bool is_transitory_false() const { return m_val == fuzzy_was_false; }   // Returns true when WasFalse.
  bool is_false() const { return m_val == fuzzy_false; }                  // Returns true when False.

  // Only use this when the value is certain.
  explicit operator bool() const { ASSERT(m_val == fuzzy_true || m_val == fuzzy_false); return m_val == fuzzy_true; }

  // Logic.

  FuzzyBool operator!() const noexcept { return FuzzyBool{static_cast<FuzzyBoolEnum>(m_val ^ bitmask)}; }
  [[gnu::always_inline]] friend inline FuzzyBool operator&&(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept;
  [[gnu::always_inline]] friend inline FuzzyBool operator||(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept;
  [[gnu::always_inline]] friend inline FuzzyBool operator!=(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept;
  [[gnu::always_inline]] friend inline FuzzyBool operator==(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept;
};

FuzzyBool operator&&(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept
{
  return FuzzyBool{static_cast<FuzzyBoolEnum>((AND_table >> (4 * fb1.m_val + fb2.m_val)) & bitmask)};
}

FuzzyBool operator||(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept
{
  return FuzzyBool{static_cast<FuzzyBoolEnum>((OR_table >> (4 * fb1.m_val + fb2.m_val)) & bitmask)};
}

FuzzyBool operator!=(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept
{
  return FuzzyBool{static_cast<FuzzyBoolEnum>((XOR_table >> (4 * fb1.m_val + fb2.m_val)) & bitmask)};
}

FuzzyBool operator==(FuzzyBoolPOD const& fb1, FuzzyBoolPOD const& fb2) noexcept
{
  return FuzzyBool{static_cast<FuzzyBoolEnum>((NOT_XOR_table >> (4 * fb1.m_val + fb2.m_val)) & bitmask)};
}

} // namespace utils
