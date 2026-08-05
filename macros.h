// SPDX-FileCopyrightText: 2015, 2017-2019, 2021-2026 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of frequently used macros.
 */

#pragma once

#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/expand.hpp>

#if defined(CWDEBUG) && !defined(__CWDS_DEBUG_H_INCLUDED)
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
#else
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_maybe_uninitialized \
  _Pragma("GCC diagnostic push")
#endif

#if defined(__GNUC__) && (!defined(__clang__) || __clang_major__ >= 11) // clang <= 10 doesn't have a -Wframe-address warning.
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_frame_address \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wframe-address\"")
#else
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_frame_address \
  _Pragma("GCC diagnostic push")
#endif

#if defined(__GNUC__) && !defined(__clang__) // clang doesn't have a -Wnon-template-friend warning.
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_non_template_friend \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wnon-template-friend\"")
#else
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_non_template_friend \
  _Pragma("GCC diagnostic push")
#endif

#if defined(__GNUC__) && defined(__clang__) // gcc doesn't have a -Wnullability-completeness.
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_nullability_completeness \
  _Pragma("GCC diagnostic push") \
  _Pragma("GCC diagnostic ignored \"-Wnullability-completeness\"")
#else
#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE_nullability_completeness \
  _Pragma("GCC diagnostic push")
#endif

#define PRAGMA_DIAGNOSTIC_PUSH_IGNORE(warn_option) \
  _Pragma("GCC diagnostic push") \
  _Pragma(BOOST_PP_STRINGIZE(GCC diagnostic ignored BOOST_PP_EXPAND(warn_option)))

#define PRAGMA_DIAGNOSTIC_POP \
  _Pragma("GCC diagnostic pop")

// Signed sizeof.
#define ssizeof (long)sizeof
