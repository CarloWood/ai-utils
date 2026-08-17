// SPDX-FileCopyrightText: 2014, 2016-2019, 2022-2023 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of cpu_relax().
 */

#pragma once

#include "macros.h"     // Incase __has_builtin is not provided by the compiler.

[[gnu::always_inline]] inline static void cpu_relax()
{
#if __has_builtin(__builtin_ia32_pause)
  __builtin_ia32_pause();
#elif defined(__i386__) || defined(__x86_64__)
  asm volatile("pause");
#elif defined(__aarch64__)
  asm volatile("yield");
#else
#error "Please extent utils/cpu_relax.h for your architecture."
#endif
}
