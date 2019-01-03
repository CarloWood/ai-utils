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
  fuzzy_true,
  fuzzy_was_true,
  fuzzy_was_false,
  fuzzy_false
};

struct FuzzyBoolPOD
{
  FuzzyBoolEnum m_val;
};

} // namespace utils

namespace fuzzy {

static constexpr FuzzyBoolPOD True = { fuzzy_true };
static constexpr FuzzyBoolPOD WasTrue = { fuzzy_was_true };
static constexpr FuzzyBoolPOD WasFalse = { fuzzy_was_false };
static constexpr FuzzyBoolPOD False = { fuzzy_false };

} // namespace fuzzy

namespace utils {

class FuzzyBool : private FuzzyBoolPOD
{
  // Uninitialized.
  FuzzyBool() { }
  FuzzyBool(FuzzyBoolPOD val) : m_val(val) { }
  FuzzyBool& operator=(FuzzyBool const& val) { m_val = val.m_val; return *this; }
  // Construct from a non-literal bool.
  explicit FuzzyBool(bool val) : m_val(val ? fuzzy_true : fuzzy_false) { }
};

} // namespace utils
