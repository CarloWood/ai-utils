#pragma once

#include <type_traits>
#include <utility>

// arrow_type<T>
//
// Evaluated to the type that T::operator-> returns,
// or to T if T itself is a pointer, or void if neither.
//
// Usage example,
//
// static_assert(std::is_same<utils::arrow_type<decltype(ptr)>, Foo*>, "ptr must be pointer-like and return Foo*");

namespace utils {

template<typename T>
concept ConceptHasArrowOperator = requires(T t) {
  { t.operator->() };
};

// The default return void.
template<typename T>
constexpr auto get_arrow_type(T p) -> void;

// If the argument is a pointer, just return that pointer type.
template<typename T>
requires std::is_pointer_v<T>
constexpr auto get_arrow_type(T* p) -> T*;

// Otherwise, if the type has operator->(), return the type that that returns.
template<typename T>
requires ConceptHasArrowOperator<T> && (!std::is_pointer_v<T>)
constexpr auto get_arrow_type(T t) -> decltype(t.operator->());

// Return the type that using `->` on T would return, or void if using `->` isn't valid.
template<typename T>
using arrow_type = decltype(get_arrow_type(std::declval<std::remove_reference_t<T>>()));

} // namespace utils
