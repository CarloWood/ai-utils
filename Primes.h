#pragma once

#include "utils/config.h"
#include "utils/macros.h"
#include <cstdint>
#include <vector>
#include <array>
#include <stdexcept>
#include <cstdlib>
#include "debug.h"

namespace utils {
namespace primes {

using prime_t = uint64_t;

static constexpr std::array<prime_t, 7> small_primes = { 2, 3, 5, 7, 11, 13, 17 };

using prime_index_t = int;            // Only used for constexpr primorial and repeat.

// Returns pₙ# = p₁⋅p₂⋅p₃… pₙ
// Note that p₀ := 1
static constexpr int calc_primorial(prime_index_t n)
{
  if (n < 1)
    return 1;
  return small_primes[n - 1] * calc_primorial(n - 1);
}

// Returns Product_{k=1..n} (pₖ-1)
// For example, repeat(4) = (p₁-1)⋅(p₂-1)⋅(p₃-1)⋅(p₄-1) = 1⋅2⋅4⋅6 = 48
static constexpr int calc_repeat(prime_index_t n)
{
  if (n < 2)
    return 1;
  return (small_primes[n - 1] - 1) * calc_repeat(n - 1);
}

// Define this to a smaller value to reduce the maximum size of sieve_word_t that will be used.
constexpr int max_sieve_word_size = 6; // 3: uint8_t, 4: uint16_t, 5: uint32_t, 6: uint64_t.

template<int compression> struct SieveWordType;
template<> struct SieveWordType<3> { using word_type = uint8_t; };
template<> struct SieveWordType<4> { using word_type = uint16_t; };
template<> struct SieveWordType<5> { using word_type = uint32_t; };
template<> struct SieveWordType<6> { using word_type = uint64_t; };

using sieve_word_t = SieveWordType<std::min(config::primes_compression_c, max_sieve_word_size)>::word_type;

} // namespace primes

class Primes
{
 public:
  using integer_t = uint64_t;
  using prime_t = primes::prime_t;
  using sieve_word_t = primes::sieve_word_t;

  static constexpr int compression = config::primes_compression_c;              // The number of primes to skip, must be less than 7 or things overflow.
  static constexpr int compression_primorial = primes::calc_primorial(compression);     // The multiplication of the first `compression` primes.
  static constexpr int compression_repeat = primes::calc_repeat(compression);           // Same, but subtracting one from each prime first.
  static constexpr int compression_first_prime = primes::small_primes[compression];     // The first integer that is not divisible by any of the `compression` primes.

  // Not storing integers in the sieve that are divisible by 2, 3 or 5.
  static constexpr int primorial = 30;  // p_5#
  static constexpr int repeat = 8;      // (3 - 1) * (5 - 1)
  static constexpr int in_sieve_mask = 0b100000100010100010100010000010;

  static integer_t calc_upper_bound_number_of_primes(integer_t n);

  static constexpr uint64_t final_sieve_size(integer_t max_value)
  {
    return repeat * ((max_value + primorial - 1) / primorial);
  }

  static std::array<unsigned char, primorial + repeat> s_table_ alignas(config::cacheline_size_c);

  static constexpr bool is_in_sieve(integer_t n)
  {
    return in_sieve_mask & (1 << (n % primorial));
  }

  static uint64_t n_to_index(integer_t n)
  {
    integer_t n_div = n / primorial;
    int n_mod = n % primorial;
    return repeat * n_div + s_table_[n_mod];
  }

  static integer_t index_to_n(uint64_t index)
  {
    uint64_t i_div = index / repeat;
    int i_mod = index % repeat;
    return primorial * i_div + s_table_[primorial + i_mod];
  }

 private:
  sieve_word_t* sieve_;
  std::vector<bool> vector_sieve_;      // index 0: 1, >0: all integers not divisible by 2, 3 or 5.
  integer_t max_value_;                 // Only integers up till and including this value can be returned and/or tested.
  int64_t index_;                       // -3: reset, -2: 2, -1: 3, 0: 5, >0: sieve index.

 public:
  Primes(integer_t max_value);

  ~Primes()
  {
    std::free(sieve_);
  }

  // Return a vector with all generated primes (don't use this when max_value is very large).
  // This function calls reset() and uses next_prime()!
  std::vector<prime_t> make_vector();

  bool is_prime(integer_t n) const
  {
    ASSERT(n <= max_value_);
    if (AI_UNLIKELY(n < 7))
      return n == 2 || n == 3 || n == 5;
    return is_in_sieve(n) && vector_sieve_[n_to_index(n)];
  }

  void reset()
  {
    index_ = -3;
  }

  prime_t next_prime()
  {
    ++index_;
    if (AI_UNLIKELY(index_ <= 0))
    {
      switch (index_)
      {
        case -2:
          return 2;
        case -1:
          return 3;
      }
      return 5;
    }
    while (!vector_sieve_[index_])
    {
      if (++index_ >= (int64_t)vector_sieve_.size())
        throw std::out_of_range("Primes: reached end of sieve.");
    }
    return index_to_n(index_);
  }

 private:
  void calculate_primes(integer_t max_value);
};

} // namespace utils
