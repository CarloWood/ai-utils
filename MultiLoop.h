/**
 * @file
 * @brief Definition of class MultiLoop.
 *
 * Copyright (C) 2011, 2017 Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <cstring>
#include <cassert>

// Implements a variable number of loops inside eachother.
// Use:
//
// // Have 23 nested for loops.
// for (MultiLoop ml(23); !ml.finished(); ml.next_loop())
//   // No code should go here!
//   for(; ml() < 3; ++ml)	// Each loops runs from 0 to 3 (in this case).
//   {
//     // Here we are at the top of loop *ml with value ml().
//     // The values of (previous) loop counters are available through ml[1..*ml]
//     // (ml() returns ml[*ml]).
//
//     if (*ml == 7 && ml[5] == 2; && ml() == 1)	// We're at the top of loop 7, the value of
//     							// loop 5 is 2 and the current loop (loop 7)
//     							// has value 1.
//     {
//       if (...)
//       {
//         ml.breaks(0);		// Normal continue (of current loop).
//         break;			// Return control to MultiLoop.
//       }
//       else if (...)
//       {
//         ml.breaks(1);		// Normal break (of current loop).
//         break;			// Return control to MultiLoop.
//       }
//       if (...)
//       {
//         ml.breaks(5);		// Break out of 5 loops.
//         break;			// Return control to MultiLoop.
//       }
//     }
//
//     if (ml.inner_loop())		// Most inner loop.
//     {
//       if (...)
//         continue;			// Normal continue of inner loop;
//         				// Calling ml.breaks(0); break; is allowed too.
//       else if (...)
//         //ml.breaks(1);		// Allowed, but doesn't have any effect.
//         break;			// Normal break from inner loop.
//
//       if (ml() == 1)			// Value of loop *ml (inner loop here) equals 1.
//       {
//         ml.breaks(5);		// break out of 5 loops.
//         break;			// Return control to MultiLoop.
//       }
//     }
//   }
//   // No code should go here!

class MultiLoop {
  public:
    // Construct a MultiLoop of n loops.
    MultiLoop(unsigned int n) : M_loops(n), M_counters(new int[n + 1]), M_current_loop(1), M_breaks(0) { M_counters[M_current_loop] = 0; }

    // Destructor.
    ~MultiLoop() { delete [] M_counters; }

    // Return the current loop number (0 ... n).
    unsigned int operator*(void) const { return M_current_loop - 1; }

    // Return the value of counter number i.
    int operator[](unsigned int i) const { assert((int)i < M_current_loop); return M_counters[i + 1]; }

    // Return a reference to counter number i.
    int& operator[](unsigned int i) { assert((int)i < M_current_loop); return M_counters[i + 1]; }

    // Return the value of the counter of the current inner loop (operator[](operator()())).
    unsigned int operator()(void) const { return M_counters[M_current_loop]; }

    // Advance the counters.
    void operator++(void);

    // Break out of the current loop and increment previous loop.
    void next_loop(void) { ++M_counters[--M_current_loop]; }

    // On the next call to next(), break out of n loops.
    void breaks(int n) { M_current_loop -= (n - 1); assert(M_current_loop > 0); }

    // Return true when all loops are finished.
    bool finished(void) const { return M_current_loop == 0; }

    // Return true when we are in the inner loop.
    bool inner_loop(void) const { return M_current_loop == (int)M_loops; }

  private:
    unsigned int  M_loops;
    int*          M_counters;
    int           M_current_loop;
    unsigned int  M_breaks;
};

inline void MultiLoop::operator++(void)
{
  // If we are not in the inner loop, start a next loop.
  if (M_current_loop < (int)M_loops)
  {
    ++M_current_loop;
    M_counters[M_current_loop] = 0;
  }
  else
  {
    // Increment current loop.
    ++M_counters[M_current_loop];
  }
}
