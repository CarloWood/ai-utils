// SPDX-FileCopyrightText: 2014, 2016-2019, 2022-2023 Carlo Wood
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
#if defined(__x86_64__) || defined(__i386__)
  asm volatile("pause" ::: "memory");
#elif defined(__aarch64__)
  asm volatile("yield" ::: "memory");
#else
#error "Please extent utils/cpu_relax.h for your architecture."
#endif
}
