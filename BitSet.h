#pragma once

namespace utils {

// POD struct for BitSetIndex.
//
// Let N = 8, 16, 32, 64 or 128 be the number of bits
// in the underlaying integral type T of the BitSet<T>
// that this index refers to (N = 8 * sizeof(T)).
//
// The meaning of m_index depends on its value.
//
// If the value is positive and in the range [0, N>, then
// it represents the bit index in the (unsigned) integral
// type T, where 0 refers to the least significant bit.
//
// If the value is negative it must be -1 (all bits set).
// This value is reserved for use in a bit iterator and means
// "one before begin()", where begin() refers to the least
// significant bit (index 0).
//
// A value of precisely N is reserved for use in a bit
// iterator and means "one past the end", or end().
//
// The use of BitSetIndexPOD is intended for constants.
// For example,
//
// constexpr BitSetIndexPOD ilsb = { 0 };
//
// could be defined to have ilsb refer to bit 0.
//
struct BitSetIndexPOD
{
  int8_t m_index;
};

// Define a few handy constants.
//
BitSetIndexPOD const index_pre_begin = { -1 };
BitSetIndexPOD const index_begin = { 0 };
template<typename T> BitSetIndexPOD const index_end = { 8 * sizeof(T) };

// Compare constants (this should never be needed, but why not add it).
constexpr bool operator==(BitSetIndexPOD i1, BitSetIndexPOD i2) { return i1.m_index == i2.m_index; }
constexpr bool operator!=(BitSetIndexPOD i1, BitSetIndexPOD i2) { return i1.m_index != i2.m_index; }

class BitSetIndex : protected BitSetIndexPOD
{
 public:
  // Constructors.

  // Construct an uninitialized BitSetIndex.
  BitSetIndex() { }
  // Copy-constructor.
  BitSetIndex(BitSetIndex const& i1) { m_index = i1.m_index; }
  // Construct a BitSetIndex from a constant.
  BitSetIndex(BitSetIndexPOD i1) { m_index = i1.m_index; }

  // Assignment operators.

  // Assign from another BitSetIndex.
  BitSetIndex& operator=(BitSetIndex i1) { m_index = i1.m_index; return *this; }
  // Assign from a constant.
  BitSetIndex& operator=(BitSetIndexPOD i1) { m_index = i1.m_index; return *this; }

  // Comparision operators.

  friend bool operator==(BitSetIndex const& i1, BitSetIndex const& i2) { return i1.m_index == i2.m_index; }
  friend bool operator==(BitSetIndex const& i1, BitSetBitSetIndexPOD i2) { return i1.m_index == i2.m_index; }
  friend bool operator==(BitSetBitSetIndexPOD i1, BitSetIndex const& i2) { return i1.m_index == i2.m_index; }
  friend bool operator!=(BitSetIndex const& i1, BitSetIndex const& i2) { return i1.m_index != i2.m_index; }
  friend bool operator!=(BitSetIndex const& i1, BitSetBitSetIndexPOD i2) { return i1.m_index != i2.m_index; }
  friend bool operator!=(BitSetBitSetIndexPOD i1, BitSetIndex const& i2) { return i1.m_index != i2.m_index; }

  friend bool operator<(BitSetIndex const& i1, BitSetIndex const& i2) { return i1.m_index < i2.m_index; }
  friend bool operator<(BitSetIndex const& i1, BitSetBitSetIndexPOD const& i2) { return i1.m_index < i2.m_index; }
  friend bool operator<(BitSetBitSetIndexPOD const& i1, BitSetIndex const& i2) { return i1.m_index < i2.m_index; }
  friend bool operator<=(BitSetIndex const& i1, BitSetIndex const& i2) { return i1.m_index <= i2.m_index; }
  friend bool operator<=(BitSetIndex const& i1, BitSetBitSetIndexPOD const& i2) { return i1.m_index <= i2.m_index; }
  friend bool operator<=(BitSetBitSetIndexPOD const& i1, BitSetIndex const& i2) { return i1.m_index <= i2.m_index; }
  friend bool operator>(BitSetIndex const& i1, BitSetIndex const& i2) { return i1.m_index > i2.m_index; }
  friend bool operator>(BitSetIndex const& i1, BitSetBitSetIndexPOD const& i2) { return i1.m_index > i2.m_index; }
  friend bool operator>(BitSetBitSetIndexPOD const& i1, BitSetIndex const& i2) { return i1.m_index > i2.m_index; }
  friend bool operator>=(BitSetIndex const& i1, BitSetIndex const& i2) { return i1.m_index >= i2.m_index; }
  friend bool operator>=(BitSetIndex const& i1, BitSetBitSetIndexPOD const& i2) { return i1.m_index >= i2.m_index; }
  friend bool operator>=(BitSetBitSetIndexPOD const& i1, BitSetIndex const& i2) { return i1.m_index >= i2.m_index; }

  // Manipulators.

  BitSetIndex& operator+=(int offset) { m_index += offset; return *this; }
  friend BitSetIndex operator+(BitSetIndex index, int offset) { BitSetIndex result(index); return result += offset; }
  friend BitSetIndex operator+(int offset, BitSetIndex index) { BitSetIndex result(index); return result += offset; }
  BitSetIndex& operator-=(int offset) { m_index -= offset; return *this; }
  friend BitSetIndex operator-(BitSetIndex index, int offset) { BitSetIndex result(index); return result -= offset; }
  friend BitSetIndex operator-(int offset, BitSetIndex index) { BitSetIndex result(index); return result -= offset; }

  BitSetIndex& operator++() { ++m_index; return *this; }
  BitSetIndex operator++(int) { BitSetIndex result(*this); operator++(); return result; }
  BitSetIndex& operator--() { ++m_index; return *this; }
  BitSetIndex operator--(int) { BitSetIndex result(*this); operator--(); return result; }

  // Accessor.

  // Return the unlaying integral value.
  int8_t operator()() const { return m_index; }

  // Special functions.

  // Advance BitSetIndex to the next bit that is set in mask.
  //
  // Index may be index_pre_begin, in which case it will be set
  // to the first bit that is set in the mask ([0, N>) if any,
  // or index_end if no bit is set.
  //
  // Otherwise BitSetIndex must be in the range [0, N>, in which
  // case a value is returned larger than the current value.
  // If no more bits could be found, Index is set to index_end.
  //
  // Don't call this function when BitSetIndex equals index_end.
  template<typename T>
  void next_bit_in(T mask)
  {
    constexpr int8_t N = index_end<T>.m_index;          // The number of bits in T, as well as 'end'.
    //                                                   ____________
    //                                                  v            \.
    // The general case, assume N == 8 and mask is 01000110 and index 2.
    //                                              ^^^^_______________________
    // In that case we require 6 to be returned.                               \.
    if (++m_index != N)                                 // m_index becomes 3.   |
    {                                                   //                     /
      mask >>= m_index;                                 // mask becomes 00001000.
      if (mask == 0)
        m_index = N;
      else
        m_index += ctz(mask);                           // m_index becomes 6.
    }
  }

  // Decrease BitSetIndex to the previous bit that is set in mask.
  //
  // Index may be index_end, in which case it will be set to the
  // last bit that is set in the mask ([0, N>) if any, or index_pre_begin
  // if no bit is set.
  //
  // Otherwise BitSetIndex must be in the range <0, N>, in which
  // case a value is returned smaller than the current value.
  // If no more bits could be found, BitSetIndex is set to index_pre_begin.
  //
  // Normally a value of 0 should always return index_pre_begin, but
  // that is not the case:
  //
  // Don't call this function when BitSetIndex equals index_begin.
  template<typename T>
  void prev_bit_in(T mask)
  {
    constexpr int8_t N = index_end<T>.m_index;          // The number of bits in T.
    //                                                _______________
    //                                               v               \.
    // The general case, assume N == 8 and mask is 01100010 and index 5.
    //                                                ^^^^______________________
    // In that case we require 1 to be returned.                                \.
    m_index = N - m_index;                              // m_index becomes 3.   /
    mask <<= m_index;                                   // mask becomes 00010000.
    if (mask == 0)
      m_index = index_pre_begin.m_index;
    else
      m_index -= clz(mask) - 1;                         // m_index becomes 1.
  }

  // Return true iff BitSetIndex is not index_pre_begin and also not 0.
  bool may_call_prev_bit_in() const { return m_index > 0; }
};

template<typename T>
class BitSet
{
  static_assert(std::is_unsigned<T>, "utils::BitSet<> template parameter must be an unsigned integral type.");

 private:
  T m_bitmask;
};

} // namespace utils
