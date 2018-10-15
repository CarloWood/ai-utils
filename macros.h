// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Declaration of frequently used macros.
//
// Copyright (C) 2017 Carlo Wood.
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

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/expand.hpp>

#if defined(CWDEBUG) && !defined(LIBCWD_SYS_H)
// We need sys.h included because that includes config.h, which defines HAVE_BUILTIN_EXPECT.
#error #include "sys.h" at the top of every source file!
#endif

#if HAVE_BUILTIN_EXPECT
#define AI_LIKELY(EXPR) __builtin_expect (static_cast<bool>(EXPR), true)
#define AI_UNLIKELY(EXPR) __builtin_expect (static_cast<bool>(EXPR), false)
#else
#define AI_LIKELY(EXPR) (EXPR)
#define AI_UNLIKELY(EXPR) (EXPR)
#endif

#define AI_CASE_RETURN(x) do { case x: return #x; } while(0)

#if defined(__GNUC__) && !defined(__clang__) // clang doesn't have a -Wmaybe-uninitialized warning.
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_maybe_uninitialized \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
#elif
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_maybe_uninitialized
#endif

#define PRAGMA_DIAGNOSTIC_PUSH_IGNORED(warn_option) \
  _Pragma("GCC diagnostic push") \
  _Pragma(BOOST_PP_STRINGIZE(GCC diagnostic ignored BOOST_PP_EXPAND(warn_option)))

#ifdef __GNUC__
#define PRAGMA_DIAGNOSTIC_POP \
  _Pragma("GCC diagnostic pop")
#endif
