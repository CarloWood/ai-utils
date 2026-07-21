// SPDX-FileCopyrightText: 2018-2019, 2022-2023, 2025 Carlo Wood
// SPDX-License-Identifier: MIT

/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class RandomStreamBuf and RandomStream.
 */

#pragma once

#include <random>
#include <concepts>

namespace utils {

// Usage:
//
// std::random_device rd;          // High-entropy source (on Linux).
// utils::RandomNumber rng(rd());  // Seed with a 64-bit random value.
//
// or call seed after default construction:
//
// utils::RandomNumber rng;
// rng.seed(rd());
//
// Draw a uniformly distributed integer from the closed range [0, 99].
//
// std::uniform_int_distribution<int> dist(0, 99);
// int value = rng.generate(dist);
//
// Draw a random number from the interval [1, 2):
//
// std::uniform_real_distribution<double> dist(1.0, 2.0);
// double value = rng.generate(dist);
//
class RandomNumber
{
 public:
  using result_type = std::mt19937_64::result_type;

 private:
  std::mt19937_64 m_twister;

 public:
  RandomNumber();

  template<class SeedSequence>
  void seed(SeedSequence& seed_sequence)
  {
    m_twister.seed(seed_sequence);
  }

  void seed(result_type seed_in)
  {
    m_twister.seed(seed_in);
  }

  template<class SeedSequence>
  RandomNumber(SeedSequence& seed_sequence)
  {
    seed(seed_sequence);
  }

  RandomNumber(result_type seed_in)
  {
    seed(seed_in);
  }

  template<std::integral INT>
  INT generate(std::uniform_int_distribution<INT>& distribution)
  {
    return distribution(m_twister);
  }

  template<std::floating_point REAL>
  REAL generate(std::uniform_real_distribution<REAL>& distribution)
  {
    return distribution(m_twister);
  }
};

} // namespace utils
