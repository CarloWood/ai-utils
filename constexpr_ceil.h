#pragma once

#include <cstdint>

// For use in static_assert.

namespace utils {

// Copied from https://stackoverflow.com/questions/31952237/looking-for-a-constexpr-ceil-function

constexpr int32_t constexpr_ceil(float num)
{
  return (static_cast<float>(static_cast<int32_t>(num)) == num)
      ? static_cast<int32_t>(num)
      : static_cast<int32_t>(num) + ((num > 0) ? 1 : 0);
}

constexpr int64_t constexpr_ceil(double num)
{
  return (static_cast<double>(static_cast<int64_t>(num)) == num)
      ? static_cast<int64_t>(num)
      : static_cast<int64_t>(num) + ((num > 0) ? 1 : 0);
}

} // namespace utils
