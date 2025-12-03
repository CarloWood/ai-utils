/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class RandomStreamBuf and RandomStream.
 *
 * @Copyright (C) 2022  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on Libera) <carlo@alinoe.com>
 * fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
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
