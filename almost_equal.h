/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of almost_equal.
 *
 * @Copyright (C) 2019  Carlo Wood.
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

#include <complex>
#include <cmath>
#include <type_traits>

namespace utils {

// Note: any comparision with NaN is false.
// So, if any number involved is NaN then almost_equal returns false.

// Return true when z1 and z2 are approximately the same.
//
// Note: if abs_relative_error is 1 then z1 and z2 are considered equal
// if their absolute value is equal and they make an angle of less than
// 90 degrees in the complex plane.
//
// More in general, let ф be the angle that z1 and z2 make in the complex
// plane and assume |z1| = |z2|, then the flipping point happens at
//
//      abs_relative_error = 2 * |z1 - z2| / |z1 + z2| = 2 * tan(ф/2) = sin(ф) * 2 / (1 + cos(ф))
//
// For most practical (small) values of abs_relative_error it is a good
// approximation to state that the flipping point angle ф = abs_relative_error,
// in radians. For example, if you'd like two complex numbers to be
// just still be considered the same when having a phase difference of
// π/16, then abs_relative_error = π/16 = 0.2 is a good choice
// (the exact value would be 2 * tan(π/32) = 0.19698280671).
//
// If on the other hand you assume that the phase of z1 and z2 are equal,
// then abs_relative_error is equal to what you'd expect when z1 and z2
// where just real numbers and
//
//      abs_relative_error = |z1 - z2| / (|z1 + z2| / 2)
//
// is indeed more like (the absolute value of) a relative error.

template<class T>
bool almost_equal(std::complex<T> z1, std::complex<T> z2, T const abs_relative_error)
{
  // Note: the std::norm returns the _square_ of the distance norm, as is
  // the case for the field norm of the Galois extension of complex numbers
  // over the real numbers (see https://en.wikipedia.org/wiki/Field_norm).
  // Hence, std::norm(z) is *not* the distance norm (||z|| = |z|) which
  // is just the magnitude of z, given by std::abs(z).
  //
  // The below return value must be read as:
  //
  //     4 |z1 - z2|² <= abs_relative_error² |z1 + z2|².
  //
  // In other words, the flipping point occurs when
  //
  //     abs_relative_error = 2 * |z1 - z2| / |z1 + z2|,
  //
  // which one could see as a standard deviation divided by a mean,
  // and therefore would be like (some definitions of) a coefficient
  // of variation (CV), but as a fraction instead of as a percentage.
  // Hence it is a unitless comparison of variability. But since we
  // only have two samples (z1 and z2) I rather speak of relative
  // error than variability. However, 'relative error' (RE) is usually
  // defined as RE(x) = (x - x0) / x0, where x0 is the "expected value",
  // or reference value and can therefore be negative. So, I added
  // the prefix 'abs_' to signify that we have the absolute value
  // of the relative error. Moreover - what is the (absolute) expected
  // value? Nothing better to use than the average I guess, so
  // z0 = |z1 + z2| / 2. And |z1 - z2| / |z1 + z2| could also be seen
  // as twice the absolute value of the 'relative error' extended to
  // complex numbers. In other words, I had a really hard time finding
  // a name for the variable...
  //
  return 4 * std::norm(z1 - z2) <= abs_relative_error * abs_relative_error * std::norm(z1 + z2);
}

// Return true when
//
//      |z1 - z2| <= (|z1 + z2| / 2) * abs_relative_error.
//
// In other words, the flipping point occurs when
//                                                         |    z1 - z2    |
//      abs_relative_error = |z1 - z2| / (|z1 + z2| / 2) = | ------------- |
//                                                         | (z1 + z2) / 2 |
//
template<class T>
typename std::enable_if<std::is_floating_point<T>::value, bool>::type
   almost_equal(T x, T y, T const abs_relative_error)
{
  return 2 * std::abs(x - y) <= abs_relative_error * std::abs(x + y);
}

} // namespace utils
