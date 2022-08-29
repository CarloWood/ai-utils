/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Implementation of the three_way_merge algorithm.
 *
 * @Copyright (C) 2011  Carlo Wood.
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

// Below we use the following notation:
//
// x (y, z) --> r
//
// where x is part of the base, y is part of left and z is part of right: base (left, right) --> result.
//
// If a different symbol is used in the same formula, then that means that the payload of that key was changed.
// When a key is non-existent in base, left or right then we use the character '-'.
// 
// Left and right are always exchangable. If x (y, z) --> r, then x (z, y) --> r.
// 
// In all cases, x (x, x) --> x,  [if neither side changes anything, nothing is changed]
//               x (x, y) --> y   [if only one side changes something, that is used]
//           and x (y, y) --> y   [if both sides make the same change, who are we to argue]
// These rules are considered trivial.
//
// For all but the trivial rules, the PayloadMerger functor is called.
// Let n, m and l be the same key but different payloads.
//
// - (-, -) --> -
// - (-, n) --> n
// - (n, -) --> n
//
// Case A:
// - (n, n) --> n
// - (n, m) --> merge payload
//
// n (-, -) --> -
//
// Case B:
// n (-, n) --> -
// n (-, m) --> merge payload
//
// Case C:
// n (n, -) --> -
// n (m, -) --> merge payload
//
// Case D:
// n (n, n) --> n
// n (n, m) --> m
// n (m, n) --> m
// n (m, m) --> m
// n (m, k) --> merge payload

template<typename InputIterator1, typename InputIterator2, typename InputIterator3,
         typename OutputIterator, typename PayloadMerger, typename Compare, typename PayloadEqual>
OutputIterator three_way_merge(InputIterator1 l, InputIterator1 le,
                               InputIterator2 b, InputIterator2 be,
                               InputIterator3 r, InputIterator3 re,
                               OutputIterator result, PayloadMerger payload_merger,
                               Compare comp, PayloadEqual payload_equal)
{
  // At any point there can be 13 ways that the smallest remaining elements are ordered:
  //
  //       Ordering            Action              Rule
  //       *b < *l < *r        ++b                 n (-, -) --> -
  //       *b < *r < *l        ++b                 n (-, -) --> -
  //       *l < *b < *r        Use *l++            - (n, -) --> n
  //       *l < *r < *b        Use *l++            - (n, -) --> n
  //       *r < *b < *l        Use *r++            - (-, n) --> n
  //       *r < *l < *b        Use *r++            - (-, n) --> n
  //
  //       *r < *b = *l        Use *r++            - (-, n) --> n
  //       *b = *l < *r        Case C, ++b, ++l
  //       *l < *b = *r        Use *l++            - (n, -) --> n
  //       *b = *r < *l        Case B, ++b, ++r
  //       *b < *l = *r        ++b                 n (-, -) --> -
  //       *l = *r < *b        Case A, ++r, ++l    - (n, m) --> n + m
  //
  //       *b = *l = *r        Case D, ++b, ++l, ++r
  //
  while (b != be || l != le || r != re)
  {
    if (b != be && (l == le || comp(*b, *l)))
    {
      // One of
      //
      //   Ordering            Action              Rule
      //   *b < *l < *r        ++b                 n (-, -) --> -
      //   *b < *r < *l        ++b                 n (-, -) --> -
      //   *r < *b < *l        Use *r++            - (-, n) --> n
      //   *b = *r < *l        Case B, ++b, ++r
      //   *b < *l = *r        ++b                 n (-, -) --> -
      //
      if (b != be && (r == re || comp(*b, *r)))
      {
        // One of
        //
        // Ordering            Action              Rule
        // *b < *l < *r        ++b                 n (-, -) --> -
        // *b < *r < *l        ++b                 n (-, -) --> -
        // *b < *l = *r        ++b                 n (-, -) --> -
        //
        // Skip it.
        ++b;
        continue;
      }
      else if (r != re && (b == be || comp(*r, *b)))
      {
        // Ordering            Action              Rule
        // *r < *b < *l        Use *r++            - (-, n) --> n
        //
        // Use it.
        *result = *r++;
        ++result;
        continue;
      }
      //   Ordering            Action              Rule
      //   *b = *r < *l        Case B, ++b, ++r
      //
      // Case B

      //if (b->second.raw_string() != r->second.raw_string())
      if (!payload_equal(*b, *r))
      {
        payload_merger(InputIterator1(), b, r, result);
      }

      ++b;
      ++r;
      continue;
    }
    // One of
    //
    //     Ordering            Action              Rule
    //     *l < *b < *r        Use *l++            - (n, -) --> n
    //     *l < *r < *b        Use *l++            - (n, -) --> n
    //     *r < *l < *b        Use *r++            - (-, n) --> n
    //     *r < *b = *l        Use *r++            - (-, n) --> n
    //     *b = *l < *r        Case C, ++b, ++l
    //     *l < *b = *r        Use *l++            - (n, -) --> n
    //     *l = *r < *b        Case A, ++r, ++l    - (n, m) --> n + m
    //     *b = *l = *r        Case D, ++b, ++l, ++r
    else if (l != le && (r == re || comp(*l, *r)))
    {
      // One of
      //
      //   Ordering            Action              Rule
      //   *l < *b < *r        Use *l++            - (n, -) --> n
      //   *l < *r < *b        Use *l++            - (n, -) --> n
      //   *b = *l < *r        Case C, ++b, ++l
      //   *l < *b = *r        Use *l++            - (n, -) --> n
      if (l != le && (b == be || comp(*l, *b)))
      {
        // One of
        //
        // Ordering            Action              Rule
        // *l < *b < *r        Use *l++            - (n, -) --> n
        // *l < *r < *b        Use *l++            - (n, -) --> n
        // *l < *b = *r        Use *l++            - (n, -) --> n
        //
        // Use it.
        *result = *l++;
        ++result;
        continue;
      }
      //   Ordering            Action
      //   *b = *l < *r        Case C, ++b, ++l
      //
      // Case C

      if (!payload_equal(*b, *l))
      {
        payload_merger(l, b, InputIterator3(), result);
      }

      ++b;
      ++l;
      continue;
    }
    // One of
    //
    //     Ordering            Action              Rule
    //     *r < *l < *b        Use *r++            - (-, n) --> n
    //     *r < *b = *l        Use *r++            - (-, n) --> n
    //     *l = *r < *b        Case A, ++r, ++l    - (n, m) --> n + m
    //     *b = *l = *r        Case D, ++b, ++l, ++r
    if (r != re && (l == le || comp(*r, *l)))
    {
      // One of
      //
      //   Ordering            Action              Rule
      //   *r < *l < *b        Use *r++            - (-, n) --> n
      //   *r < *b = *l        Use *r++            - (-, n) --> n
      //
      // Use it.
      *result = *r++;
      ++result;
      continue;
    }
    // One of
    //
    //     Ordering            Action              Rule
    //     *l = *r < *b        Case A, ++r, ++l    - (n, m) --> n + m
    //     *b = *l = *r        Case D, ++b, ++l, ++r
    if (r != re && (b == be || comp(*r, *b)))
    {
      //   Ordering            Action              Rule
      //   *l = *r < *b        Case A, ++r, ++l    - (n, m) --> n + m
      //
      // Case A
      //
      if (payload_equal(*r, *l))
      {
        // - (n, n) --> n
        *result = *r;
        ++result;
      }
      else
      {
        // - (n, m) --> merge payload
        payload_merger(l, InputIterator2(), r, result);
      }

      ++r;
      ++l;
      continue;
    }
    //     Ordering            Action
    //     *b = *l = *r        Case D, ++b, ++l, ++r
    //
    // Case D
    // n (n, n) --> n
    // n (n, m) --> m
    // n (m, n) --> m
    // n (m, m) --> m
    // n (m, l) --> merge payload

    if (payload_equal(*b, *r))
    {
      // One of
      // n (n, n) --> n
      // n (m, n) --> m
      *result = *l;
      ++result;
    }
    else if (payload_equal(*b, *l))
    {
      // n (n, m) --> m
      *result = *r;
      ++result;
    }
    else if (payload_equal(*l, *r))
    {
      // n (m, m) --> m
      *result = *r;
      ++result;
    }
    else
    {
      // n (m, k) --> merge payload
      payload_merger(l, b, r, result);
    }

    ++b;
    ++l;
    ++r;
  }

  return result;
}
