// SPDX-FileCopyrightText: 2019, 2022, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class HasherStreamBuf and StreamHasher.
 */

#pragma once

// Calculate the hash of a stream of characters.
//
// Example usage:
//
//   utils::StreamHasher hasher;
//   utils::RandomStreamBuf random_streambuf(1000, 'A', 'Z');
//   hasher << &random_streambuf;
//   ASSERT(hasher.digest() == 0xf373022bdeab5158);

#include <ostream>
#include <array>
#include <algorithm>
#include <tuple>
#include <boost/functional/hash.hpp>

namespace utils {

class HasherStreamBuf : public std::streambuf
{
 private:
  size_t m_hash;
  std::array<char, 64> m_buf;   // The resulting hash value is a function of the size of this array!
  static constexpr size_t bufsize = std::tuple_size_v<decltype(m_buf)>;

  void add_and_reset_put_area()
  {
    if (pptr() > pbase())
      boost::hash_combine(m_hash, boost::hash_range(pbase(), pptr()));
    setp(&m_buf[0], m_buf.data() + bufsize);
  }

 protected:
  int_type overflow(int_type c) override
  {
    if (c != EOF)
    {
      if (pptr() == epptr())
        add_and_reset_put_area();
      *pptr() = c;
      pbump(1);
    }
    return 0;
  }

 public:
  HasherStreamBuf() : m_hash(0) { setp(&m_buf[0], m_buf.data() + bufsize); }

  size_t hash()
  {
    add_and_reset_put_area();
    return m_hash;
  }

  struct size_hash_pair_t {
    size_t size;
    size_t hash;
  };

  // For streams with characters in the range ['A', 'Z'].
  // These hashes correspond with boost version 1.88.0.
  static constexpr std::array<size_hash_pair_t, 9> size_hash_pairs = {{
    { 1, 0x59b6c7886c0aac05 },
    { 10, 0x3886ca52d4567a76 },
    { 100, 0xda7559ed38282f2b },
    { 1000, 0xf8654124c32cbce3 },
    { 10000, 0xef11f1d322998371 },
    { 100000, 0xa756fdb74970eae8 },
    { 1000000, 0x5690483fc4413fb5 },
    { 10000000, 0x1e31aa56afb5d78b },
    { 100000000, 0x4f406899b456059a }
  }};
};

class StreamHasher : public std::ostream
{
 private:
  HasherStreamBuf m_streambuf;

 public:
  StreamHasher() { rdbuf(&m_streambuf); }
  ~StreamHasher() { }

  size_t digest() { return m_streambuf.hash(); }
};

} // namespace utils
