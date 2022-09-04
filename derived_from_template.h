#include <utility>
#include <type_traits>

// With many thanks to Alipha from IRC <3.
namespace utils {

// derived_from_template
//
// Usage:
//
#if -0
template<typename T>
struct B {};

template<typename T>
struct C : B<T> {};

struct D {};

template<typename T>
struct E {};

template<typename T>
struct F : E<T> {};

struct G : B<int> {};

struct H : B<int>, B<long> {};

int main()
{
  std::cout << std::boolalpha << derived_from_template_v<C<int>, B> << std::endl;       // true
  std::cout << std::boolalpha << derived_from_template_v<D, B> << std::endl;            // false
  std::cout << std::boolalpha << derived_from_template_v<E<int>, B> << std::endl;       // false
  std::cout << std::boolalpha << derived_from_template_v<G, B> << std::endl;            // true
  std::cout << std::boolalpha << derived_from_template_v<int, B> << std::endl;          // false
  std::cout << std::boolalpha << derived_from_template_v<H, B> << std::endl;            // false (should have been true)
}
#endif

namespace detail {
template<template<typename...> class BaseTemplate, typename... Args>
void derived_tester(BaseTemplate<Args...>*) {}
} // namespace detail

// Only works with public inheritance.
// Remove const/ref qualifiers before use.
// Doesn't work if Derived inherits publicly from multiple classes of BaseTemplate.
template<typename Derived, template<typename...> class BaseTemplate, typename = void>
struct derived_from_template
{
  static constexpr bool value = false;
};

template<typename Derived, template<typename...> class BaseTemplate>
struct derived_from_template<Derived, BaseTemplate, std::void_t<decltype(detail::derived_tester<BaseTemplate>(std::declval<Derived*>()))>>
{
  static constexpr bool value = true;
};

template<typename Derived, template<typename...> class BaseTemplate>
constexpr bool derived_from_template_v = derived_from_template<Derived, BaseTemplate>::value;

} // namespace utils
