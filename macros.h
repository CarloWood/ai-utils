#pragma once

#if defined(__GNUC__) && __GNUC__ >= 3
#define AI_LIKELY(EXPR) __builtin_expect (!!(EXPR), true)
#define AI_UNLIKELY(EXPR) __builtin_expect (!!(EXPR), false)
#else
#define AI_LIKELY(EXPR) (EXPR)
#define AI_UNLIKELY(EXPR) (EXPR)
#endif

#define UNUSED_ARG(x)

#if __SIZEOF_SIZE_T__ == __SIZEOF_LONG__
#define CW_FORMAT_SIZE_T "l"
#else
#define CW_FORMAT_SIZE_T "ll"
#endif
