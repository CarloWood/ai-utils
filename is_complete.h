#include <type_traits>

namespace utils {

// Test if a template parameter was already completely defined.
//
// Usage:
//
//   static_assert(is_complete_v<int>, "Huh? int is complete type!");
//   static_assert(is_complete_v<CompleteType>, "CompleteType should be a complete type.");
//   static_assert(!is_complete_v<IncompleteType>, "IncompleteType should not be a complete type.");
//
template <typename T>
auto is_complete_impl(int) -> decltype(sizeof(T), std::true_type{})
{
  return std::true_type{};
}

template <typename T>
auto is_complete_impl(...) -> std::false_type
{
  return std::false_type{};
}

template <typename T>
struct is_complete : decltype(is_complete_impl<T>(0))
{
};

template <typename T>
inline constexpr bool is_complete_v = is_complete<T>::value;

} // namespace utils
