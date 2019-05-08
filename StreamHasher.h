// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class HasherStreamBuf and StreamHasher.
//
// Copyright (C) 2019 Carlo Wood.
//
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file is part of ai-utils.
//
// ai-utils is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ai-utils is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

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
};

class StreamHasher : public std::ostream
{
 private:
  HasherStreamBuf m_streambuf;

 public:
  StreamHasher() { rdbuf(&m_streambuf); }
  size_t digest() { return m_streambuf.hash(); }
};

} // namespace utils

#ifdef EXAMPLE_CODE
int main()
{
  utils::RandomStreamBuf random(10000, 'a', 'z'); // Create a streambuf that produces 100 pseudo-random characters in the interval ['a', 'z'].
  utils::StreamHasher hasher;
  hasher << &random;
  std::cout << std::hex << hasher.digest() << '\n';
}
#endif
