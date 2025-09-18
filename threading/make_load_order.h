#pragma once

#include <atomic>

namespace utils::threading {

// This function "removes" the 'release' part of `success_order` in the same
// way as `compare_exchange_weak` and `compare_exchange_strong`, that take only
// a single memory order as argument, do to construct the `failure_order`.
//
// The idea of this function is to use it for the initial load prior to such calls.
// For example:
//
//   int expected = value.load(utils::threading::make_load_order(success_order));
//   do
//   {
//     ...
//   }
//   while (!value.compare_exchange_weak(expected, target_value, success_order));
//
// If the compare_exchange fails then the new value is loaded into expected
// using the same memory order that this function returns.
//
[[gnu::always_inline]] inline constexpr std::memory_order make_load_order(std::memory_order success_order) noexcept
{
  switch (success_order)
  {
    case std::memory_order_release:
      return std::memory_order_relaxed;
    case std::memory_order_acq_rel:
      return std::memory_order_acquire;
    default:
      return success_order;  // seq_cst stays seq_cst, acquire stays acquire, etc.
  }
}

} // namespace utils::threading
