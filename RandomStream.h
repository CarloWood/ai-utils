// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of class RandomStreamBuf and RandomStream.
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

#include <istream>
#include <array>
#include <algorithm>
#include <random>

namespace utils {

class RandomStreamBuf : public std::streambuf
{
 private:
  size_t m_size;
  std::array<char, 64> m_buffer;
  std::mt19937_64 m_twister;
  std::uniform_int_distribution<char> m_dist;

 protected:
  int_type underflow() override
  {
    if (m_size == 0)
      return EOF;

    size_t size = std::min(m_size, m_buffer.size());
    setg(&m_buffer[0], &m_buffer[0], &m_buffer[size]);
    for (size_t i = 0; i < size; ++i)
      m_buffer[i] = m_dist(m_twister);
    m_size -= size;
    return 0;
  }

 public:
  RandomStreamBuf(size_t size, char b, char e) : m_size(size), m_dist(b, e) { }
};

class RandomStream : public std::istream
{
 private:
  RandomStreamBuf m_random_streambuf;

 public:
  RandomStream(size_t size, char b, char e) : m_random_streambuf(size, b, e) { rdbuf(&m_random_streambuf); }
  ~RandomStream() { }
};

} // namespace utils
