/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class HasherStreamBuf and StreamHasher.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This file is part of ai-utils.
 *
 * ai-utils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ai-utils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.
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
    setp(&m_buf[0], &m_buf[bufsize]);
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
  HasherStreamBuf() : m_hash(0) { setp(&m_buf[0], &m_buf[bufsize]); }

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
  static constexpr std::array<size_hash_pair_t, 9> size_hash_pairs = {{
    { 1, 0xaedc04cfa2e5b999 },
    { 10, 0xa32fa7216c0d9b4b },
    { 100, 0xd82a1e09aeb1383 },
    { 1000, 0xf373022bdeab5158 },
    { 10000, 0x345360b3a8fa2b73 },
    { 100000, 0x4755f0873f1b649 },
    { 1000000, 0x1e6fca364672c0d },
    { 10000000, 0xd8ef85366ad39522 },
    { 100000000, 0x737dddb5f390f1aa }
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
