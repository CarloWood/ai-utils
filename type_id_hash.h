#pragma once

#include <string_view>
#include <cstdint>

namespace utils {
namespace type_id_hash_details {

// Instantiate one template function per type, that returns a unique
// constexpr identifier: a character string with the name of this function.
template<typename T>
constexpr static std::string_view type_id_string()
{
  return {__PRETTY_FUNCTION__};
}

// Helper function to constexpr hash the above string.
using hash_t = uint64_t;
static constexpr hash_t fnv_basis = 14695981039346656037ull;
static constexpr hash_t fnv_prime = 1099511628211ull;
static constexpr hash_t fnv1a_hash(std::string_view sv, hash_t hash = fnv_basis)
{
  return sv.size() > 0 ? fnv1a_hash({sv.data() + 1, sv.size() - 1}, (hash ^ sv[0]) * fnv_prime) : hash;
}

} // namespace type_id_details

// Returns a uint64_t that can be used as unique identifier for the type T.
// Is guaranteed to return the same hash value during runtime when invoked
// with the same type T. Different hash value should be returned for different
// types.
template<typename T>
consteval auto type_id_hash()
{
  return type_id_hash_details::fnv1a_hash(type_id_hash_details::type_id_string<T>());
}

} // namespace utils
