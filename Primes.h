#pragma once

#include "utils/config.h"
#include "utils/macros.h"
#include <cstdint>
#include <vector>
#include <array>
#include <stdexcept>
#include <cstdlib>
#include <numeric>
#include <climits>
#include "debug.h"

namespace utils {
namespace primes {

using prime_t = uint64_t;       // Used for primes.
using integer_t = uint64_t;     // Used for (large) integers that might be primes.
using prime_index_t = int;      // Only used for constexpr primorial and repeat.

static constexpr std::array<prime_t, 7> small_primes = { 2, 3, 5, 7, 11, 13, 17 };

// Returns pₙ# = p₁⋅p₂⋅p₃… pₙ
// Note that p₀ := 1
constexpr int calc_primorial(prime_index_t n)
{
  if (n < 1)
    return 1;
  return small_primes[n - 1] * calc_primorial(n - 1);
}

// Returns Product_{k=1..n} (pₖ-1)
// For example, repeat(4) = (p₁-1)⋅(p₂-1)⋅(p₃-1)⋅(p₄-1) = 1⋅2⋅4⋅6 = 48
constexpr int calc_repeat(prime_index_t n)
{
  if (n < 2)
    return 1;
  return (small_primes[n - 1] - 1) * calc_repeat(n - 1);
}

// Define this to a smaller value to reduce the maximum size of sieve_word_t that will be used.
constexpr int max_sieve_word_size = 6; // 3: uint8_t, 4: uint16_t, 5: uint32_t, 6: uint64_t.

constexpr int compression_repeat = calc_repeat(config::primes_compression_c);   // Number of columns in the sieve.

template<int compression> struct SieveWordType;
template<> struct SieveWordType<3> { using word_type = uint8_t; };
template<> struct SieveWordType<4> { using word_type = uint16_t; };
template<> struct SieveWordType<5> { using word_type = uint32_t; };
template<> struct SieveWordType<6> { using word_type = uint64_t; };

using sieve_word_t = SieveWordType<std::min(config::primes_compression_c, max_sieve_word_size)>::word_type;

// If, for example, compression_repeat == 13 (which is never the case, but assume it was)
// and sieve_word_t is one byte, we'd have the following situation:
//
//                     111
// Column: 01234567  89012
//        [11111111][11111000] <-- One row of words_per_row (2) sieve_word_t of which the first three bits are unused (repeat = 13).
//         --------       --- <-- unused bits (3)
//         \_ sieve_word_bits = 8

constexpr int sieve_word_bits = sizeof(sieve_word_t) * CHAR_BIT;                                // The number of bits in one sieve_word_t.
constexpr int words_per_row = (compression_repeat + sieve_word_bits - 1) / sieve_word_bits;     // The width of one row in sieve_word_t.
constexpr int unused_bits = words_per_row * sieve_word_bits - compression_repeat;               // Number of unused bits at the end of a row.
// It's too much work to support unused bits (and would needlessly slow down the algorithm).
static_assert(unused_bits == 0);

} // namespace primes

class Primes
{
 public:
  using integer_t = primes::integer_t;
  using prime_t = primes::prime_t;
  using sieve_word_t = primes::sieve_word_t;

  static constexpr int compression = config::primes_compression_c;              // The number of primes to skip, must be less than 7 or things overflow.
  static constexpr int compression_primorial = primes::calc_primorial(compression);     // The multiplication of the first `compression` primes.
  static constexpr int compression_first_prime = primes::small_primes[compression];     // The first integer that is not divisible by any of the `compression` primes.
  static constexpr integer_t small_primes_primorial = 614889782588491410ULL;            // p₁₅#, the largest primorial that fits in an integer_t.
  static constexpr integer_t small_primes_mask = 0x8a20a08a28acULL;                     // A mask with a bit set for all small primes up to 47.

  static integer_t calc_upper_bound_number_of_primes(integer_t n);

  static constexpr int n_to_row0_to_column_index(integer_t n)
  {
    return ((n - compression_first_prime) % compression_primorial + (compression_first_prime % 3)) / 3;
  }

  static constexpr int row0_to_column_size = compression_primorial / 3 + 1;
  static std::array<uint16_t, row0_to_column_size> const row0_to_column;   // Needed for is_prime.

 private:
  sieve_word_t* sieve_;
  integer_t max_value_;                 // Only integers up till and including this value can be returned and/or tested.
  integer_t sieve_rows_;                // The number of rows of sieve_.
  int64_t index_;                       // -3: reset, -2: 2, -1: 3, 0: 5, >0: sieve index.
  int64_t index_end_;                   // Size of the sieve in bits.

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
    integer_t gcd_with_primorial = std::gcd(small_primes_primorial, n);
    // Is n divisible by 2, 3, 5, 7, 11, 13, 17, ..., 43 or 47?
    if (gcd_with_primorial != 1)
    {
      // In that case n is prime iff it is equal to one of those primes.
      return (n < 64) ? (small_primes_mask >> n) & 1 : false;
    }
    if (AI_UNLIKELY(n == 1))
      return false;
    // n is not divisible by any of the skipped primes. Do a sieve look-up.
    int row    = (n - compression_first_prime) / compression_primorial;
    int column = row0_to_column[(n % compression_primorial) / 3];
    unsigned int col_word_offset =                     column / primes::sieve_word_bits;
    sieve_word_t col_mask        = sieve_word_t{1} << (column % primes::sieve_word_bits);
    unsigned int wi = row + col_word_offset * sieve_rows_;

    return *(sieve_ + wi) & col_mask;
  }

  void reset()
  {
    index_ = -compression - 1;
  }

  prime_t next_prime();

 private:
  void calculate_primes(integer_t max_value);
};

} // namespace utils
