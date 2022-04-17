/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of class ColorPool.
 *
 * @Copyright (C) 2021  Carlo Wood.
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

#include "log2.h"
#include <cstdint>
#include <array>
#include <type_traits>

// class ColorPool
//
// Provides colors from a pool (integer values in the range [0, number_of_colors>)
// that haven't been used the longest time.
//
// Usage:
//
// utils::ColorPool<24> cp;     // Only return values in the range [0, 24>.
//
// int color = cp.get_color();  // Returns a color that hasn't been used yet, or
//                              // that hasn't been used for the longest time.
// cp.use_color(color);         // Actually use the color.
//
// use_color(color) may be called for any color in the range [0, 24> as many times
// as you want; it keeps automatically track of when a color was used last.
// get_color() will return the color that wasn't use for the longest time.
// Note that get_color() will keep returning the same color until you use it.

namespace utils {

template<int number_of_colors>
class ColorPool
{
 public:
  static_assert(0 < number_of_colors && number_of_colors <= 256, "number_of_colors must be in the range [1, 256].");

  static constexpr int bits = ceil_log2(static_cast<unsigned int>(number_of_colors));
  using indexpair_type = typename std::conditional<bits <= 4, uint8_t, uint16_t>::type;
  static constexpr int width = sizeof(indexpair_type) << 2;
  static constexpr indexpair_type prev_mask = static_cast<indexpair_type>(-(1U << width));      // PREV0000
  static constexpr indexpair_type next_mask = static_cast<indexpair_type>(~-(1U << width));     // 0000NEXT
  using index_type = indexpair_type;

  static index_type next(indexpair_type index_pair) { return index_pair & next_mask; }
  static index_type prev(indexpair_type index_pair) { return index_pair >> width; }
  static indexpair_type index_to_next(index_type index) { return index; }
  static indexpair_type index_to_prev(index_type index) { return index << width; }
  static indexpair_type prev_to_next(indexpair_type index_pair) { return index_pair >> width; }
  static indexpair_type next_to_prev(indexpair_type index_pair) { return index_pair << width; }
  static indexpair_type combine(index_type prev, index_type next) { return (prev << width) | next; }

// private:
  alignas(config::cacheline_size_c) std::array<indexpair_type, number_of_colors> history;
  int m_next_color{};

 public:
  void use_color(int color);
  int get_color() const { return m_next_color; }

  int get_and_use_color()
  {
    int nc = m_next_color;
    m_next_color = next(history[nc]);
    return nc;
  }

  ColorPool();
};

template<int number_of_colors>
ColorPool<number_of_colors>::ColorPool()
{
  indexpair_type last_pair = combine(number_of_colors - 2, 0);
  for (index_type i = 0; i < history.size(); ++i)
    last_pair = history[i] =
      combine((prev(last_pair) + 1) % number_of_colors,
              (next(last_pair) + 1) % number_of_colors);
}

template<int number_of_colors>
void ColorPool<number_of_colors>::use_color(int color)
{
  if (color == m_next_color)
  {
    m_next_color = next(history[m_next_color]);
    return;
  }

  // We have the following in the history:
  //
  // index   prev/next
  //    pi:    xx|CC   = hp
  // color:    pi|ni   = color_PN
  //    ni:    CC|yy   = hn
  //
  // This must become (after removal of 'color'):
  //
  //    pi:    xx|ni
  // color:    pi|ni
  //    ni:    pi|yy
  //
  // Load the history into L1 cache.
  indexpair_type color_PN = history[color];
  // Decode it.
  index_type ni = next(color_PN);
  // If this is already the last color to use, then we don't need to do anything.
  if (ni == m_next_color)
    return;
  index_type pi = prev(color_PN);
  // Prepare the bits that we need to write to remove this node from the list.
  indexpair_type  prev_0N = index_to_next(ni);  // 00|ni
  indexpair_type  next_P0 = index_to_prev(pi);  // pi|00
  // Read the other history values that we need.
  indexpair_type hp = history[pi];
  indexpair_type hn = history[ni];
  // Actually remove the color node:
  history[pi] = (hp & prev_mask) | prev_0N;     // xx|ni
  history[ni] = (hn & next_mask) | next_P0;     // pi|yy

  // Furthermore, we have the following in the history (where nc = m_next_color):
  //
  // index   prev/next
  //    pi:    xx|nc   = hp
  //    nc:    pi|yy   = hn
  //
  // which has to become (after insertion of 'color'):
  //
  //    pi:    xx|CC
  // color:    pi|nc
  //    nc:    CC|yy
  //
  // Read the remaining history values that we need:
  hn = history[m_next_color];
  pi = prev(hn);
  hp = history[pi];
  // Prepare the bits that we need to write to remove this node from the list.
  prev_0N = index_to_next(color);  // 00|CC
  next_P0 = index_to_prev(color);  // CC|00
  // Actually insert the color node:
  history[m_next_color] = (hn & next_mask) | next_P0;
  history[pi] = (hp & prev_mask) | prev_0N;
  history[color] = combine(pi, m_next_color);
}

} // namespace utils
