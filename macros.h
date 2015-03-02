#if defined(__GNUC__) && __GNUC__ >= 3
#define AI_LIKELY(EXPR) __builtin_expect (!!(EXPR), true)
#define AI_UNLIKELY(EXPR) __builtin_expect (!!(EXPR), false)
#else
#define AI_LIKELY(EXPR) (EXPR)
#define AI_UNLIKELY(EXPR) (EXPR)
#endif
