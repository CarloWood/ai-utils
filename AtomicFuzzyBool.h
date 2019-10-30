// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of AtomicFuzzyBool.
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

#include "FuzzyBool.h"
#include <atomic>

namespace utils {

class AtomicFuzzyBool
{
 private:
   std::atomic_int m_aval;
   static_assert(std::atomic<FuzzyBoolEnum>::is_always_lock_free, "Get a real OS.");

 public:
  AtomicFuzzyBool() : m_aval(0) { }
  AtomicFuzzyBool(FuzzyBoolPOD const& val) : m_aval(val.m_val) { }

  void store(FuzzyBoolPOD const& desired, std::memory_order order = std::memory_order_seq_cst) noexcept { m_aval.store(desired.m_val, order); }
  FuzzyBool operator=(FuzzyBoolPOD const& val) { m_aval = val.m_val; return val; }

  FuzzyBool load(std::memory_order order = std::memory_order_seq_cst) const noexcept { return FuzzyBool{static_cast<FuzzyBoolEnum>(m_aval.load(order))}; }
  explicit operator FuzzyBool() const { return FuzzyBool{static_cast<FuzzyBoolEnum>(m_aval.load())}; }

  void print_on(std::ostream& os, std::memory_order order = std::memory_order_seq_cst) const { os << m_aval.load(order); }
  friend std::ostream& operator<<(std::ostream& os, AtomicFuzzyBool const& aval) { aval.print_on(os); return os; }

  friend FuzzyBool operator==(AtomicFuzzyBool const& aval, FuzzyBoolPOD const& val) { return aval.load() == val; }
  friend FuzzyBool operator==(FuzzyBoolPOD const& val, AtomicFuzzyBool const& aval) { return val == aval.load(); }
  friend FuzzyBool operator!=(AtomicFuzzyBool const& aval, FuzzyBoolPOD const& val) { return aval.load() != val; }
  friend FuzzyBool operator!=(FuzzyBoolPOD const& val, AtomicFuzzyBool const& aval) { return val != aval.load(); }

  bool is_true(std::memory_order order = std::memory_order_seq_cst) const { return m_aval.load(order) == fuzzy_true; }                  // Returns true when True.
  bool is_transitory_true(std::memory_order order = std::memory_order_seq_cst) const { return m_aval.load(order) == fuzzy_was_true; }   // Returns true when WasTrue;
  bool is_momentary_true(std::memory_order order = std::memory_order_seq_cst) const { return m_aval.load(order) & fuzzy_was_true; }     // Returns true when WasTrue or True.
  bool is_momentary_false(std::memory_order order = std::memory_order_seq_cst) const { return !(m_aval.load(order) & fuzzy_was_true); } // Returns true when WasFalse or False.
  bool is_transitory_false(std::memory_order order = std::memory_order_seq_cst) const { return m_aval.load(order) == fuzzy_was_false; } // Returns true when WasFalse;
  bool is_false(std::memory_order order = std::memory_order_seq_cst) const { return m_aval.load(order) == fuzzy_false; }                // Returns true when False.

  FuzzyBool fetch_invert(std::memory_order order = std::memory_order_seq_cst)
  {
    return FuzzyBool{static_cast<FuzzyBoolEnum>(m_aval.fetch_xor(fuzzy_true, order))};  // Returns the old value.
  }

  FuzzyBool fetch_AND(FuzzyBoolPOD const& val, std::memory_order order = std::memory_order_seq_cst)
  {
    int expected = m_aval.load(std::memory_order_relaxed);
    while (!m_aval.compare_exchange_weak(expected, (AND_table >> (4 * expected + val.m_val)) & 0xc, order))
      ;  // The body of this loop is empty.
    return FuzzyBool{static_cast<FuzzyBoolEnum>(expected)};
  }

  FuzzyBool fetch_OR(FuzzyBoolPOD const& val, std::memory_order order = std::memory_order_seq_cst)
  {
    int expected = m_aval.load(std::memory_order_relaxed);
    while (!m_aval.compare_exchange_weak(expected, (OR_table >> (4 * expected + val.m_val)) & 0xc, order))
      ;  // The body of this loop is empty.
    return FuzzyBool{static_cast<FuzzyBoolEnum>(expected)};
  }

  FuzzyBool fetch_XOR(FuzzyBoolPOD const& val, std::memory_order order = std::memory_order_seq_cst)
  {
    int expected = m_aval.load(std::memory_order_relaxed);
    while (!m_aval.compare_exchange_weak(expected, (XOR_table >> (4 * expected + val.m_val)) & 0xc, order))
      ;  // The body of this loop is empty.
    return FuzzyBool{static_cast<FuzzyBoolEnum>(expected)};
  }

  FuzzyBool fetch_NOT_XOR(FuzzyBoolPOD const& val, std::memory_order order = std::memory_order_seq_cst)
  {
    int expected = m_aval.load(std::memory_order_relaxed);
    while (!m_aval.compare_exchange_weak(expected, (NOT_XOR_table >> (4 * expected + val.m_val)) & 0xc, order))
      ;  // The body of this loop is empty.
    return FuzzyBool{static_cast<FuzzyBoolEnum>(expected)};
  }
};

} // namespace utils
