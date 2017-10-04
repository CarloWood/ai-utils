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
// Usage:
//
// // Have 23 nested for loops.
// for (MultiLoop ml(23); !ml.finished(); ml.next_loop())
// {
//   // No code should go here!
//   while (ml() < 3)	        // Each loops runs from 0 to 3 (in this case).
//   {
//     // Here we are at the top of loop *ml with value ml().
//     // The values of (previous) loop counters are available through ml[0..*ml]
//     // (ml() returns ml[*ml]).
//
//     // Beginning of loop code (all loops).
//     if (*ml == 7 && ml[5] == 2 && ml() == 1)         // We're at the top of loop number 7 (the 8th loop),
//                                                      // the value of loop #5 is 2 and the current loop (loop 7)
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
//       // Inner loop body.
//
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
//
//       // End of inner loop.
//     }
//     // Loop start values.
//     ml.start_next_loop_at(ml());     // This causes each loop to begin at with the value of the previous loop.
//   }
//
//   if (int loop = ml.end_of_loop(); loop >= 0)
//   {
//     // End of loop code for loop 'loop'.
//   }
// }

class MultiLoop {
  public:
    // Construct a MultiLoop of n loops.
    explicit MultiLoop(unsigned int n, int b = 0) :
      M_loops(n),
      M_counters(new int[n + 1]),
      M_current_loop(n > 0 ? 1 : 0),
      M_continued(false)
      {
        M_counters[M_current_loop] = b;
      }

    // Destructor.
    ~MultiLoop() { delete [] M_counters; }

    // Return the current loop number (0 ... n-1).
    unsigned int operator*(void) const { return M_current_loop - 1; }

    // Return the value of counter number i.
    int operator[](unsigned int i) const { assert((int)i < M_current_loop); return M_counters[i + 1]; }

    // Return a reference to counter number i.
    int& operator[](unsigned int i) { assert((int)i < M_current_loop); return M_counters[i + 1]; }

    // Return the value of the counter for the n-th previous loop.
    int operator()(unsigned int n = 0) const { assert((int)n < M_current_loop); return M_counters[M_current_loop - n]; }

    // Set the loop counter to value n.
    void operator=(int n) { M_counters[M_current_loop] = n; }

    // Advance the counters. Start the next loop with value b.
    void start_next_loop_at(int b = 0);

    // Break out of the current loop and increment previous loop.
    void next_loop(void) { ++M_counters[--M_current_loop]; M_continued = false; }

    // On the next call to start_next_loop_at(), break out of n loops.
    // A value of n == 0 means a `continue` of the current loop.
    void breaks(int n) { M_continued = !n; M_current_loop -= (n - 1); assert(M_current_loop > 0); }

    // Return true when all loops are finished.
    bool finished(void) const { return M_current_loop == 0; }

    // Return true when we are in the inner loop.
    bool inner_loop(void) const { return M_current_loop == (int)M_loops; }

    // Return true when we're at the end of a loop (but not the inner loop).
    int end_of_loop() const { return M_continued ? -1 : M_current_loop - 2; }

  private:
    unsigned int  M_loops;
    int*          M_counters;
    int           M_current_loop;
    bool          M_continued;
};

inline void MultiLoop::start_next_loop_at(int b)
{
  // If we are not in the inner loop, start a next loop.
  if (M_current_loop < (int)M_loops)
  {
    ++M_current_loop;
    M_counters[M_current_loop] = b;
  }
  else
  {
    // Increment current loop.
    ++M_counters[M_current_loop];
  }
}
