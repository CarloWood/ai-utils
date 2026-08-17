// SPDX-FileCopyrightText: 2014, 2016-2019, 2022-2023 Carlo Wood
// SPDX-FileCopyrightText: 2026 Godina
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Declaration of cpu_relax().
 */

#pragma once

[[gnu::always_inline]] inline static void cpu_relax()
{
#if defined(__aarch64__)
  // AArch64 YIELD is the architectural alias of HINT #1.
  __asm__ __volatile__("hint #1" ::: "memory");
#elif defined(__x86_64__) || defined(__i386__)
  asm volatile("pause" ::: "memory");
#else
#error "Please extent utils/cpu_relax.h for your architecture."
#endif
}
