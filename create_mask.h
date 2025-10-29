#pragma once

#include <type_traits>
#include <utility>
#include <limits>
#include <climits>      // CHAR_BIT

namespace utils {

//=============================================================================
// integer_promotion / integer_promotion_t
//
// Convert integral type T to the type that it will become when doing some arithmetic with it, like shifting.
//

template<typename T>
struct integer_promotion
{
  static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "T must be an unsigned integral type");

  // The promoted type is what you get when you apply unary + to the type.
  using type = decltype(+std::declval<T>());
};

template<typename T>
using integer_promotion_t = typename integer_promotion<T>::type;

// End of integer_promotion / integer_promotion_t
//=============================================================================

//=============================================================================
// create_mask
//
// Create a mask of integral type `coordinates_type` with the `bits` number of least significant bits set.
//

template<typename coordinates_type, int bits>
static constexpr coordinates_type create_mask()
{
  // The type that coordinates_type would be promoted to when shifting it.
  using mask_type = integer_promotion_t<coordinates_type>;
  // Create a mask with the same number of 1's as are bits in coordinates_type.
  mask_type const ones = std::numeric_limits<coordinates_type>::max();
  // The total number of (least significant) bits that are set in `ones`.
  int const total_bits = sizeof(coordinates_type) * CHAR_BIT;
  static_assert(0 <= bits && bits <= total_bits, "The shift is out of range for coordinates_type!");
  mask_type const mask = ones >> (total_bits - bits);
  return mask;
}

// End of create_mask
//=============================================================================

} // namespace utils
