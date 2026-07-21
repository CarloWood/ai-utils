// SPDX-FileCopyrightText: 2018-2019, 2022, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class RandomStreamBuf and RandomStream.
 */

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
    setg(&m_buffer[0], &m_buffer[0], m_buffer.data() + size);
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
