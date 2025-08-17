// This is a copy of https://github.com/alipha/cpp/blob/8c4313a8ba131e4822c5adcd09e62dc2b2ab9c3d/endian/endian.hpp
// See https://github.com/alipha/cpp/blob/8c4313a8ba131e4822c5adcd09e62dc2b2ab9c3d/LICENSE for the license.

// Change log
//
// 2025/08/14
// - Put everything in namespace utils.
// - Added utils::ntoh and utils::hton.
// - Use #pragma once instead of macro guard.

#pragma once

#include <cstdint>
#include <type_traits>

namespace utils {

/* gcc 8.1 and clang 9.0.0 and above optimize these to nops or bswaps with -O2.
   However, le_to_uint16 and be_to_uint16 are not optimal on gcc 8.1-9.3, but
   are optimal on 10.1 and above.
   Also, clang 9.0.0 produces non-optimal code for be_to_uint64 and le_to_uint64
   if the -march=native flag is provided. clang 10.0.0 and above are optimal.
   The excessive static_casts are to quiet -Wconversion -Wsign-conversion warnings.
 */
template<typename Char>
Char *uint64_to_be(std::uint64_t src, Char *dest) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    dest[0] = static_cast<Char>(static_cast<std::uint8_t>(src >> 56));
    dest[1] = static_cast<Char>(static_cast<std::uint8_t>(src >> 48));
    dest[2] = static_cast<Char>(static_cast<std::uint8_t>(src >> 40));
    dest[3] = static_cast<Char>(static_cast<std::uint8_t>(src >> 32));
    dest[4] = static_cast<Char>(static_cast<std::uint8_t>(src >> 24));
    dest[5] = static_cast<Char>(static_cast<std::uint8_t>(src >> 16));
    dest[6] = static_cast<Char>(static_cast<std::uint8_t>(src >> 8));
    dest[7] = static_cast<Char>(static_cast<std::uint8_t>(src));
    return dest;
}

template<typename Char>
Char *uint64_to_le(std::uint64_t src, Char *dest) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    dest[7] = static_cast<Char>(static_cast<std::uint8_t>(src >> 56));
    dest[6] = static_cast<Char>(static_cast<std::uint8_t>(src >> 48));
    dest[5] = static_cast<Char>(static_cast<std::uint8_t>(src >> 40));
    dest[4] = static_cast<Char>(static_cast<std::uint8_t>(src >> 32));
    dest[3] = static_cast<Char>(static_cast<std::uint8_t>(src >> 24));
    dest[2] = static_cast<Char>(static_cast<std::uint8_t>(src >> 16));
    dest[1] = static_cast<Char>(static_cast<std::uint8_t>(src >> 8));
    dest[0] = static_cast<Char>(static_cast<std::uint8_t>(src));
    return dest;
}

template<typename Char>
std::uint64_t be_to_uint64(const Char *src) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    return static_cast<std::uint64_t>(
        static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[0])) << 56
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[1])) << 48
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[2])) << 40
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[3])) << 32
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[4])) << 24
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[5])) << 16
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[6])) << 8
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[7]))
    );
}

template<typename Char>
std::uint64_t le_to_uint64(const Char *src) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    return static_cast<std::uint64_t>(
        static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[7])) << 56
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[6])) << 48
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[5])) << 40
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[4])) << 32
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[3])) << 24
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[2])) << 16
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[1])) << 8
        | static_cast<std::uint64_t>(static_cast<std::uint8_t>(src[0]))
    );
}


template<typename Char>
Char *uint32_to_be(std::uint32_t src, Char *dest) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    dest[0] = static_cast<Char>(static_cast<std::uint8_t>(src >> 24));
    dest[1] = static_cast<Char>(static_cast<std::uint8_t>(src >> 16));
    dest[2] = static_cast<Char>(static_cast<std::uint8_t>(src >> 8));
    dest[3] = static_cast<Char>(static_cast<std::uint8_t>(src));
    return dest;
}

template<typename Char>
Char *uint32_to_le(std::uint32_t src, Char *dest) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    dest[3] = static_cast<Char>(static_cast<std::uint8_t>(src >> 24));
    dest[2] = static_cast<Char>(static_cast<std::uint8_t>(src >> 16));
    dest[1] = static_cast<Char>(static_cast<std::uint8_t>(src >> 8));
    dest[0] = static_cast<Char>(static_cast<std::uint8_t>(src));
    return dest;
}

template<typename Char>
std::uint32_t be_to_uint32(const Char *src) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    return static_cast<std::uint32_t>(
        static_cast<std::uint32_t>(static_cast<std::uint8_t>(src[0])) << 24
        | static_cast<std::uint32_t>(static_cast<std::uint8_t>(src[1])) << 16
        | static_cast<std::uint32_t>(static_cast<std::uint8_t>(src[2])) << 8
        | static_cast<std::uint32_t>(static_cast<std::uint8_t>(src[3]))
        );
}

template<typename Char>
std::uint32_t le_to_uint32(const Char *src) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    return static_cast<std::uint32_t>(
        static_cast<std::uint32_t>(static_cast<std::uint8_t>(src[3])) << 24
        | static_cast<std::uint32_t>(static_cast<std::uint8_t>(src[2])) << 16
        | static_cast<std::uint32_t>(static_cast<std::uint8_t>(src[1])) << 8
        | static_cast<std::uint32_t>(static_cast<std::uint8_t>(src[0]))
    );
}


template<typename Char>
Char *uint16_to_be(std::uint16_t src, Char *dest) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    dest[0] = static_cast<Char>(static_cast<std::uint8_t>(src >> 8));
    dest[1] = static_cast<Char>(static_cast<std::uint8_t>(src));
    return dest;
}

template<typename Char>
Char *uint16_to_le(std::uint16_t src, Char *dest) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    dest[1] = static_cast<Char>(static_cast<std::uint8_t>(src >> 8));
    dest[0] = static_cast<Char>(static_cast<std::uint8_t>(src));
    return dest;
}

template<typename Char>
std::uint16_t be_to_uint16(const Char *src) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(static_cast<std::uint8_t>(src[0])) << 8
        | static_cast<std::uint16_t>(static_cast<std::uint8_t>(src[1]))
    );
}

template<typename Char>
std::uint16_t le_to_uint16(const Char *src) {
    static_assert(sizeof(Char) == 1, "Char must be a byte-sized type");
    return static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(static_cast<std::uint8_t>(src[1])) << 8
        | static_cast<std::uint16_t>(static_cast<std::uint8_t>(src[0]))
    );
}

// Reworked from https://stackoverflow.com/a/77937931/1487069
template<typename T> std::enable_if_t<sizeof(T) == 1, T> ntoh(T v) { return v; }
template<typename T> std::enable_if_t<sizeof(T) == 2, T> ntoh(T v) { return be_to_uint16(reinterpret_cast<char const*>(&v)); }
template<typename T> std::enable_if_t<sizeof(T) == 4, T> ntoh(T v) { return be_to_uint32(reinterpret_cast<char const*>(&v)); }
template<typename T> std::enable_if_t<sizeof(T) == 8, T> ntoh(T v) { return be_to_uint64(reinterpret_cast<char const*>(&v)); }

template<typename T> std::enable_if_t<sizeof(T) == 1, T> hton(T v) { return v; }
template<typename T> std::enable_if_t<sizeof(T) == 2, T> hton(T v) { T result; uint16_to_be(v, reinterpret_cast<char*>(&result)); return result; }
template<typename T> std::enable_if_t<sizeof(T) == 4, T> hton(T v) { T result; uint32_to_be(v, reinterpret_cast<char*>(&result)); return result; }
template<typename T> std::enable_if_t<sizeof(T) == 8, T> hton(T v) { T result; uint64_to_be(v, reinterpret_cast<char*>(&result)); return result; }

} // namespace utils
