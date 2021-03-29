/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class ColorPool.
 *
 * @Copyright (C) 2021  Carlo Wood.
 *
 * pub   dsa3072/C155A4EEE4E527A2 2018-08-16 Carlo Wood (CarloWood on freenode) <carlo@alinoe.com>
 * Primary key fingerprint: 8020 B266 6305 EE2F D53E  6827 C155 A4EE E4E5 27A2
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

#include "log2.h"
#include <cstdint>

// class ColorPool
//
// Provides colors from a pool (integer values in the range [0, number_of_colors>)
// that haven't been used the longest time.
//
// Usage:
//
// utils::ColorPool<6> cp;      // Only return values in the range [0, 6>.
//
// int color = cp.get_color();  // Returns a color that hasn't been used yet, or
//                              // that hasn't been used for the longest time.
// cp.use_color(color);         // Actually use the color.
//
// get_color() will keep returning the same color of course, until you use it.
// use_color may be called for any color in the range [0, 6> as many times as
// you want; it keeps automatically track of which color hasn't been used for
// the longest time, which then will be returned by get_color().

namespace utils {

namespace cs {
static constexpr uint64_t one = 1;
static constexpr uint64_t ones = 0x0101010101010101ULL;
static constexpr uint64_t fake_history = 0x0102040810204080ULL;
static constexpr uint64_t color2mask(int color) { return one << color; }
} // namespace cs

template<int number_of_colors>
class ColorPool
{
  static_assert(0 < number_of_colors && number_of_colors <= 8, "number_of_colors must be in the range [1, 8].");

 private:
  uint64_t history;

 public:
  void use_color(int color);
  int get_color();

  ColorPool() : history(cs::fake_history >> 8 * (8 - number_of_colors)) { }
};

template<int number_of_colors>
void ColorPool<number_of_colors>::use_color(int color)
{
  uint64_t where = cs::ones << color;
  uint64_t color_in_history = history & where;
  uint64_t rotate_mask = color_in_history - 1;
  uint64_t needs_rotation = history & rotate_mask;
  rotate_mask |= color_in_history;
  history &= ~rotate_mask;
  history |= needs_rotation << 8;
  history |= cs::color2mask(color);
}

template<int number_of_colors>
int ColorPool<number_of_colors>::get_color()
{
  return log2(history >> (8 * (number_of_colors - 1)));
}

} // namespace utils
