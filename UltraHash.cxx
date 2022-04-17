#include "sys.h"
#include "UltraHash.h"
#include "log2.h"
#include "MultiLoop.h"
#include "AIAlert.h"
#include "debug.h"
#include <algorithm>
#include <bit>
#include <iostream>
#include <iomanip>

#include <bitset>

namespace utils {

void transpose(std::array<uint64_t, 64>& M_dst, std::array<uint64_t, 64> const& M_src)
{
  // It's magic.
  uint64_t* dst = M_dst.data();
  uint64_t const* src = M_src.data();
  uint64_t m = 0xffffffff;
  int j_rs_dst = 64;
  int j_rs_src = 32;
  uint64_t* const end = dst + j_rs_dst;
  int j = 32;
  j_rs_dst >>= 1;
  uint64_t* wk = dst;
  for (uint64_t const* wks = src; wk < end; wk += j_rs_dst, wks += j_rs_src)
    for (int k = 0; k < j; ++k, ++wk, ++wks)
    {
      uint64_t d = ((*wks >> j) ^ *(wks + j_rs_src)) & m;
      *wk = *wks ^ (d << j);
      *(wk + j_rs_dst) = *(wks + j_rs_src) ^ d;
    }
  m ^= m << 16;
  for (j = 16; j != 0; j = j >> 1, m ^= m << j)
  {
    j_rs_dst >>= 1;
    for (wk = dst; wk < end; wk += j_rs_dst)
      for (int k = 0; k < j; ++k, ++wk)
      {
        uint64_t d = ((*wk >> j) ^ *(wk + j_rs_dst)) & m;
        *wk ^= d << j;
        *(wk + j_rs_dst) ^= d;
      }
  }
}

uint64_t just_last_bit(uint64_t n)
{
  return ((n ^ (n - 1)) >> 1) + 1;
}

// This function checks if the n given keys (in K) are linear independent and
// if they are - fills in Mᵀ with the magic numbers to convert the keys to a
// number in the range [0, 64> (64 --> 6 bits); the perfect hash.
//
// This involves solving a linear (matrix) equation of the form:
//
// K M = L
//
// Where each row of K is a 64-bit key, M is to be solved, and L exists
// of rows that are - in fact - the values returned by UtraHash::index:
// 6 bit numbers.
//
// For example, let K be a 3x5 matrix (3 keys of 5 bits):
//
//      Column 0 (lsb)
//         |
//         v
//       ⎡ 1 1 0 1 1 ⎤  <-- row 0 = key 1
//   K = ⎢ 1 1 1 0 1 ⎥              key 2
//       ⎣ 1 1 1 1 0 ⎦              key 3
//
// Note that column 0 is stored in the least significant bit of the uint64_t,
// the uint64_t values of the keys are therefore 27, 23 and 15, in this example.
//
// In order to visualize the relationship with the std::array<uint64_t, sz>
// variables we'll put column 0 on the right, but then also invert the rows
// in order to keep an indentity matrix visually unchanged (rotate the whole
// thing 180 degrees).
//
//              Column 0 (lsb)
//                 |
//                 v
//       ⎡ 0 1 1 1 1 ⎤             K[2] = 15
//   K = ⎢ 1 0 1 1 1 ⎥             K[1] = 23
//       ⎣ 1 1 0 1 1 ⎦  <-- row 0, K[0] = 27
//
// Furthermore, let L be such that it simply contains each row number in order:
//
//        Column 0 (lsb)
//           |
//           v
//       ⎡ 1 0 ⎤      row 2: 2
//   L = ⎢ 0 1 ⎥      row 1: 1
//       ⎣ 0 0 ⎦  <-- row 0: 0
//
// Note that values in the rows of L don't really matter, as long as they
// are all different.
//
// The matrix equation K M = L reads, in this example,
//
//                 ⎡ z e ⎤ <-- msb
//   ⎡ 0 1 1 1 1 ⎤ ⎢ y d ⎥   ⎡ 1 0 ⎤
//   ⎢ 1 0 1 1 1 ⎥ ⎢ x c ⎥ = ⎢ 0 1 ⎥
//   ⎣ 1 1 0 1 1 ⎦ ⎢ w b ⎥   ⎣ 0 0 ⎦
//                 ⎣ v a ⎦
//                     ^
//                     |
//                  index 0
//
// where edcba is the binary representation of the value in the first position of MT, etc.
// Namely, if we transpose M, we get:
//
//                lsb (column 0 of Mᵀ; row 0 of M).
//                 |
//                 v
//        ⎡z y x w v ⎤
//   Mᵀ = ⎣e d c b a ⎦ <-- MT[0] (row 0 of Mᵀ / column 0 of M).
//
// The matrix equation remains the same if we substract row i from row j (i != j)
// in both K and L, provided all rows in both matrices are different.
//
// If the keys are linearly independent we'll be able to put K
// in row echelon form (K') where each next row has more zeroes in
// the leading columns (or since we put the matrix upside down,
// in the trailing colums):
//
// For example, subtracting (XOR-ing) the bottom row (row 0) from row 1 and 2,
// and then substracting row 1 from row 2, gives
//
//                     ⎡ z e ⎤
//       ⎡ 1 1 0 0 0 ⎤ ⎢ y d ⎥   ⎡ 1 1 ⎤
// K' =  ⎢ 0 1 1 0 0 ⎥ ⎢ x c ⎥ = ⎢ 0 1 ⎥
//       ⎣ 1 1 0 1 1 ⎦ ⎢ w b ⎥   ⎣ 0 0 ⎦
//                     ⎣ v a ⎦
//
// Where each row (starting at the bottom) has more zeroes on the
// right than the previous row (the row below it).
//
// This already proves that the keys were linearly independent,
// because there is no row that has all zeroes in K and not in L.
// Note that even if there was a row in K with all zeroes, we might
// still be able to find a solution if also L has all zeroes in that row.
//
// The above is corresponding to the set of equations:
//                         ⎡ 1 1 0 ⎤
//                    Lᵀ = ⎣ 1 0 0 ⎦
//    1 1 0 0 0
//    e+d                =   1
//    z+y                =   1
//
//    0 1 1 0 0
//      d+c              =     1
//      y+x              =     0
//
//    1 1 0 1 1
//    e+d  +b+a          =       0
//    z+y  +w+v          =       0
//
// and from top to bottom we want to
//
// 1) set d to 1 (keep e at 0).
//    set y to 1 (keep z at 0).
// 2) set c to 1 + d.
//    set x to 0 + y.
// 3) set a to 0 + e + d (keep b at 0).
//    set v to 0 + z + y (keep w at 0).
//
// It is easy to verify that this answer also holds in the original
// equation:
//                     1 0 <---- 1 0 <-- column m.
//   .-- row n         v v       | |    .-- row n.
//   v               ⎡ 0 0 ⎤     v v    v
//   2 ⎡ 0 1 1 1 1 ⎤ ⎢ 1 1 ⎥   ⎡ 1 0 ⎤  2
//   1 ⎢ 1 0 1 1 1 ⎥ ⎢ 1 0 ⎥ = ⎢ 0 1 ⎥  1
//   0 ⎣ 1 1 0 1 1 ⎦ ⎢ 0 0 ⎥   ⎣ 0 0 ⎦  0
//                   ⎣ 1 1 ⎦
//
// That is, the bit in L at row n and column m is the parity of
// the XOR between the key at row n in K and column m of M.
//
// Finally we transpose this result to get the output variable MT:
//
//        ⎡ 0 1 1 0 1 ⎤
//   Mᵀ = ⎣ 0 1 0 0 1 ⎦
//
// Since a zero key always results in a zero in L, we can not use
// the value zero in L when one of the keys in zero. Instead we
// ignore the key and use different and non-zero values in L.
//
// For example, if we had the same keys as in the above example,
// but ALSO an all zeroes key:
//
//       ⎡ 1 1 0 1 1 ⎤  <-- row 0 = key 1
//       ⎢ 0 0 0 0 0 ⎥              key 2
//   K = ⎢ 0 1 1 0 0 ⎥              key 3
//       ⎣ 0 1 1 1 1 ⎦              key 4
//
// Then the zero key is removed and we use for L the matrix
//
//       ⎡ 1 1 ⎤
//   L = ⎢ 1 0 ⎥
//       ⎣ 0 1 ⎦
//
// The algorithm to solve M from K' in row echelon form is, as you can see
// from the 1) 2) 3) steps above, done per whole row of M:
//
// First, we start with M being all zeroes:
//
// n=5      row
// ⎡ 0 0 ⎤  4
// ⎢ 0 0 ⎥  3
// ⎢ 0 0 ⎥  2
// ⎢ 0 0 ⎥  1
// ⎣ 0 0 ⎦  0
//
// We skip over row 4, leaving it all zeroes, because the first row of
// K' is 1 1 0 0 0
//       4 3 <-- start with row 3.
//         ^-- the last 1.
// And that row is made equal to the first row of L: 1 1.
// The result after step 1) is therefore:
//
// n=5      row
// ⎡ 0 0 ⎤  4
// ⎢ 1 1 ⎥  3   <-- last updated.
// ⎢ 0 0 ⎥  2
// ⎢ 0 0 ⎥  1
// ⎣ 0 0 ⎦  0
//
// The next row of K' is 0 1 1 0 0
//                       4 3 2 <-- continue with row 2.
//                           ^-- the last 1.
// And that row is made equal to L's second row [ 0 1 ]
// XOR the current row of K' times M so far:
//
//               ⎡ 0 0 ⎤
//               ⎢ 1 1 ⎥
// [ 0 1 1 0 0 ] ⎢ 0 0 ⎥ = [ 1 1 ]
//               ⎢ 0 0 ⎥
//               ⎣ 0 0 ⎦
//
// [ 0 1 ] + [ 1 1 ] = [ 1 0 ]
//
// So that after step 2) M has become:
//
// n=5      row
// ⎡ 0 0 ⎤  4
// ⎢ 1 1 ⎥  3
// ⎢ 1 0 ⎥  2   <-- last updated.
// ⎢ 0 0 ⎥  1
// ⎣ 0 0 ⎦  0
//
// The final row of K' is 1 1 0 1 1
//                        4 3 2 1 0 <-- continue with row 0.
//                                ^-- the last 1.
// The next row of L is [ 0 0 ] and
//
//                         ⎡ 0 0 ⎤
//                         ⎢ 1 1 ⎥
// [ 0 0 ] + [ 1 1 0 1 1 ] ⎢ 1 0 ⎥ = [ 1 1 ]
//                         ⎢ 0 0 ⎥
//                         ⎣ 0 0 ⎦
//
// So that after step 3) M has become:
//
// n=5      row
// ⎡ 0 0 ⎤  4
// ⎢ 1 1 ⎥  3
// ⎢ 1 0 ⎥  2
// ⎢ 0 0 ⎥  1
// ⎣ 1 1 ⎦  0   <-- last updated.
//
bool UltraHash::create_set(std::array<uint64_t, 6>& MT, std::array<uint64_t, 64>& K, int n)
{
  // Detect if we have a zero key.
  bool has_zero_key = false;
  for (int i = 0; i < n; ++i)
    if (K[i] == 0)
    {
      has_zero_key = true;
      K[i] = K[--n];            // Remove the zero key.
      break;
    }

  // Construct L.
  std::array<uint64_t, 64> L;
  for (int i = 0; i < n; ++i)
    L[i] = i + has_zero_key;
  for (int i = n; i < 64; ++i)
    L[i] = 0;

  // Swipe K and L.
  //
  // At this point during swiping:
  //
  //     0 0 1 0 0 0   <-- bit_mask
  //   ⎡ 1 1 0 0 0 0 ⎤
  //   ⎢ 1 1 1 0 0 0 ⎥
  //   ⎢ 0 1 1 0 0 0 ⎥
  //   ⎢ 1 1 1 0 0 0 ⎥ <-- target_row
  //   ⎢ 1 0 1 0 1 0 ⎥
  //   ⎣ 1 1 1 0 0 1 ⎦ <-- row 0
  //           ^ ^ ^
  //           / | \__
  //  Columns 2, 1 and 0 are already swiped. Now bit_mask will be '0 0 1 0 0 0'
  //  and target_row will be '2'.
  //
  // bit_mask: the column that is currently being swiped.
  // target_row: the row of K that needs to contain this bit.
  int target_row = 0;
  for (uint64_t bit_mask = 1; bit_mask; bit_mask <<= 1)
  {
    // Look for the largest row that has bit_mask set, that is not smaller than target_row.
    int pivot = -1;
    for (int row = n - 1; row >= target_row; --row)
      if ((K[row] & bit_mask))
      {
        pivot = row;
        break;
      }
    if (pivot == -1)                    // No such row - this doesn't mean the equation can't be solved yet; so continue with the next column.
      continue;

    if (!(K[target_row] & bit_mask))
    {
      K[target_row] ^= K[pivot];        // Set the bit in the target row.
      L[target_row] ^= L[pivot];
    }

    // Continue updating the remaining rows till target_row.
    for (int row = pivot; row > target_row; --row)
    {
      if ((K[row] & bit_mask))
      {
        K[row] ^= K[target_row];
        L[row] ^= L[target_row];

        if (K[row] == 0 && L[row] != 0)
          return false;                 // No solution.
      }
    }
    ++target_row;
  }

  // Fill MT with all zeroes.
  MT = {};

  // Solve MT from the echelon form of K and L.
  for (int KL_row = n - 1; KL_row >= 0; --KL_row)
  {
    int MT_col = std::countr_zero(K[KL_row]);
    uint64_t MT_col_bit = uint64_t{1} << MT_col;
    int L_col = 5;
    for (uint64_t L_col_bit = uint64_t{1} << L_col; L_col_bit; --L_col, L_col_bit >>= 1)
      MT[L_col] ^= (!(L[KL_row] & L_col_bit) != !parity(K[KL_row] & MT[L_col])) ? MT_col_bit : 0;
  }

  return true;
}

int UltraHash::initialize(std::vector<uint64_t> const& keys)
{
  size_t const number_of_keys = keys.size();
  DoutEntering(dc::notice, "UltraHash::initialize(<vector with " << number_of_keys << " keys>)");

  // If the keys are well hashed, then statistically we should have
  // on average 62.18 contigous linearly independent keys, with a
  // maximum of 64 of course (the keys live in a 64 dimensional space).
  //
  // That is, we should be able put the first 62 or so in a linear
  // independent set; then the next 62 etc. But sometimes we might
  // have less contigous keys that are linearly independent that way.
  //
  // If we do not simply pick the first 64 keys in `keys`, but instead
  // pick the first 64 keys that have a particular bit set, then we
  // have 64 different chances to get a good, statistically representative,
  // distribution and actually will be able to parition the keys in
  // sets with an average size of ~62.
  //
  // Therefore, we expect to need ceil(number_of_keys / 62) different sets.
  // The best case scenario is that we can find linearly independent
  // sets each of size 64; therefore the minimum number of bits required
  // is.
  int const minimum_number_of_bits = std::bit_width((number_of_keys + 63U) / 64U - 1U);
  // If that isn't working then we'll try one bit more, which should
  // always work (statistically; or you should hash your keys better).

  // Reserve a scratch area on the stack.
  std::array<uint64_t, 64> M{};

  // The first thing we're going to do is fill M[i] with the count of
  // ones that appear as bit i in the list of keys:
  //
  //  msb                            lsb
  //   |                              |
  //   v                              v
  //   000...00000000000000000000110111  <-- M[63] number of keys that have bit 63 set: 0b110111
  //                   .                      .
  //                   .                      .                  etc.
  //   000...00000000000000000000010111  <-- M[3]  number of keys that have bit 3 set:   0b10111
  //   000...00000000000000000000110101  <-- M[2]  number of keys that have bit 2 set:  0b110101
  //   000...00000000000000000001001011  <-- M[1]  number of keys that have bit 1 set: 0b1001011
  //   000...00000000000000000000110101  <-- M[0]  number of keys that have bit 0 set:  0b110101
  //
  // The transpose function mirrors this in the diagonal from lsb/i=0 to msb/i=63:
  //
  //                                 1  ⎞
  //   1                           1101 ⎟
  //   1                           0101 ⎟
  //   0...........................1010 ⎟-- The count of keys that have that bits set
  //   1...........................1101 ⎟   (the count is vertically with the lsb at the bottom).
  //   1...........................1010 ⎟
  //   1...........................1111 ⎠
  //                               ^^^^
  //                               ||||
  //                            ...3210 <-- bit number that is set in keys.
  //
  // This transposed form can be generated in parallel, so lets do that:
  for (uint64_t key : keys)
  {
    int i = 0;
    uint64_t carry;
    do
    {
      carry = M[i] & key;      // Calculate the next carry.
      M[i] ^= key;             // Fix M[i] for adding key to the vertical counts.
      key = carry;             // In case we have a carry and return to the top of the do..while loop.
      ++i;
    }
    while (carry);
  }
  transpose(M, M);              // Transpose is "fast" - does 6 times a full matrix operation.
                                // So much faster than any single-bit manipulations would ever be.

  // Having these counts we expect them to be set half of the time (that would
  // be the best for partitioning the keys into sets); thus, number_of_keys / 2.
  // Therefore replace the counts with the square of (count - number_of_keys / 2),
  // and also left shift them by 32 places, to put them in the high 32 bit of M.
  // The lower 32 bit are then filled with a counter:
  //
  //  msb                                                             lsb
  //   |                                                               |
  //   v                                                               v
  //                 .          squares                 .  counter
  //                 .             |                    .     |
  //                 .             v                    .     v
  //   00000000000000000000000001011010 00000000000000000000000000000100 <-- M[4]
  //   00000000000000000000000000010111 00000000000000000000000000000011 <-- M[3]
  //   00000000000000000000000000100010 00000000000000000000000000000010 <-- M[2]
  //   00000000000000000000000000011101 00000000000000000000000000000001 <-- M[1]
  //   00000000000000000000000000110101 00000000000000000000000000000000 <-- M[0]
  //
  for (int i = 0; i < 64; ++i)
  {
    int diff = M[i] - number_of_keys / 2;
    uint64_t square = diff * diff;
    M[i] = square << 32 | i;
  }
  // Now sort M, which will give us a list of bits in the low 32 bits that
  // are ordered from a count close to half to larger and larger deviations.
  std::sort(M.begin(), M.end());
  // This gives a list of bits indices in the lower 32bits of M that we want to
  // try, starting at M[0], getting worse for larger indices of M.
  // Instead of the bit index, lets store them as a mask; which is also the format
  // that the bits that we'll use for real, in the end, will be stored in m_b.
  std::array<uint64_t, 64> bit_mask;
  for (int i = 0; i < 64; ++i)
  {
    uint32_t bit_index = M[i];
    bit_mask[i] = uint64_t{1} << bit_index;
  }

  bool found_sets = false;
  // Try if we can find lineary independent sets using just minimum number of
  // bits required. If it fails, we try with one bit more (which really should
  // just always work).
  int number_of_bits = minimum_number_of_bits;
  while (number_of_bits <= max_test_bits && !found_sets)
  {
    // Using this many bits, the number of sets we'll have is
    int const number_of_sets = 1 << number_of_bits;

    if (number_of_bits == 0)
    {
      std::array<uint64_t, 64> set0{};
      int sz = keys.size();
      for (int i = 0; i < sz; ++i)
        set0[i] = keys[i];
      found_sets = create_set(m_M_sets[0], set0, sz);
      if (found_sets)
      {
        for (int b = 0; b < max_test_bits; ++b)
          m_b[b] = 0;
        break;
      }
      ++number_of_bits;
      continue;
    }

    // Next, we try that number of bits bit_mask's close to the beginning first.
    // In other words, we need number_of_bits for loops where the outer
    // loop runs from 0 till 63, and each subsequent loop runs from 0 till the
    // value of the previous loop.
    //
    // Having three loops that would give:
    //   2 1 0
    //   3 1 0
    //   3 2 0
    //   3 2 1  <-- use bit_mask[3], bit_mask[2] and bit_mask[1]
    //   4 1 0
    //   4 2 0
    //   4 2 1
    //   4 3 0
    //   4 3 1
    //   4 3 2
    //   and so on.
    //
    // I am gonna go ahead and use one of the hardest to understand API
    // that I wrote: MultiLoop-- but hey, once it works, it works in general,
    // also for larger number of bits; so it might be worth it.
    //
    // Start `number_of_bits` for loops.
    for (MultiLoop ml(number_of_bits); !ml.finished(); ml.next_loop())
    {
      // The first loop (when *ml == 0) ends at 63, other loops end just before the current value[] of the previous loop.
      while (ml() < (*ml == 0 ? 64 : ml[*ml - 1]))
      {
        if (ml.inner_loop())
        {
          // Using these bits (bit_mask[ml[0..number_of_bits]]) we're going
          // partition the keys into number_of_sets sets of linearly independent
          // keys, if possible.
          std::vector<std::array<uint64_t, 64>> key_sets(number_of_sets);
          std::vector<int> ki(number_of_sets);  // Current key index.
          // Sanity check.
          for (int i = 0; i < number_of_sets; ++i)
            ASSERT(ki[i] == 0);
          // First put the bit masks in m_b so that we can use set_index().
          for (int b = 0; b < max_test_bits; ++b)
            m_b[b] = b < number_of_bits ? bit_mask[ml[b]] : 0;
          // Next, partition the keys.
          bool too_many_keys_in_one_set = false;
          for (uint64_t key : keys)
          {
            int si = set_index(key);
            // Paranoia check; should never fail.
            ASSERT(si < number_of_sets);
            key_sets[si][ki[si]++] = key;
            // More than 64 keys can never be linear independent so we don't even try.
            if ((too_many_keys_in_one_set = ki[si] > size_t{64}))
              break;
          }
          if (too_many_keys_in_one_set)
          {
            ml.breaks(0);       // continue
            break;
          }

          // Sanity check.
          for (int i = 0; i < number_of_sets; ++i)
            ASSERT(ki[i] <= 64);

          // We found bits that allowed us to partition the keys in sets of size 64 or smaller.
          // If the keys in any set are not linearly independent then we'll still reject this combination.
          found_sets = true;            // Be optimistic.

          // Try to create the 6x64 matrices in m_M_sets.
          for (int si = 0; si < number_of_sets; ++si)
            if (!create_set(m_M_sets[si], key_sets[si], ki[si]))
            {
              found_sets = false;
              break;
            }
          if (!found_sets)
          {
            ml.breaks(0);       // continue
            break;
          }

          Dout(dc::notice|continued_cf, "Found the following " << number_of_bits << " working bits:");
          for (int l = 0; l < number_of_bits; ++l)
            Dout(dc::continued, ' ' << ml[l]);
          Dout(dc::finish, ".");

          // Break out of *all* loops.
          ml.breaks(number_of_bits);
          break;

        }
        ml.start_next_loop_at(0);
      }
    }
    if (!found_sets)
      ++number_of_bits;
  }

  if (!found_sets)
    THROW_ALERT("Too many keys ([KEYS])! UltraHash was written for ~100 keys, but should work up till [WORKS] keys.", AIArgs("[KEYS]", number_of_keys)("[WORKS]", 50 * m_M_sets.size()));

  return 1 << (6 + number_of_bits);
}

} // namespace utils
