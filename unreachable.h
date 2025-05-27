// utils/unreachable.h
//
// Implement std::reachable for C++ versions older than 23.
// Tested for C++11 (needed for [[noreturn]]) and up on several compilers.

#pragma once

// If the compiler has the new std::unreachable already, use it.
#ifdef __cpp_lib_unreachable
#include <utility>              // std::unreachable
#else
namespace std {
// From https://stackoverflow.com/a/65258501/1487069
#ifdef __GNUC__ // GCC 4.8+, Clang, Intel and other compilers compatible with GCC (-std=c++0x or above).
[[noreturn]] inline __attribute__((always_inline)) void unreachable() {__builtin_unreachable();}
#elif defined(_MSC_VER) // MSVC
[[noreturn]] __forceinline void unreachable() {__assume(false);}
#else // ???
inline void unreachable() {}
#endif
} // namespace std
#endif
