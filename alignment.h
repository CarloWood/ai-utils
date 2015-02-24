/**
 * \file alignment.h
 * \brief Definition of alignment related utilities.
 *
 * Copyright (C) 2015 Aleric Inglewood.
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

#ifndef UTILS_ALIGNMENT_H
#define UTILS_ALIGNMENT_H

template<int size>
struct size_to_type
{
  typedef long long integral_t;
};

#if __SIZEOF_LONG__ < __SIZEOF_LONG_LONG__
template<>
struct size_to_type<sizeof(long)>
{
  typedef long integral_t;
}
#endif

#if __SIZEOF_INT__ < __SIZEOF_LONG__
template<>
struct size_to_type<sizeof(int)>
{
  typedef int integral_t;
};
#endif

#if __SIZEOF_SHORT__ < __SIZEOF_INT__
template<>
struct size_to_type<sizeof(short int)>
{
  typedef short int integral_t;
};
#endif

#if 1 < __SIZEOF_SHORT__
template<>
struct size_to_type<sizeof(char)>
{
  typedef char integral_t;
};
#endif

template<int x>
struct ilog2
{
  enum { value = (1 + ilog2<x/2>::value) };
};

template<>
struct ilog2<1>
{
  enum { value = 0 };
};

// Calculate alignment.
//
// SIZE: the size of the type (struct) that needs alignment.
// max_size must be a power of two and less than or equal sizeof(long long).
//
// alignment<SIZE>::log2 is the log base 2 of SIZE (rounded down to nearest integer).
// alignment<SIZE>::size is SIZE rounded down to the nearest power of 2.
// alignment<SIZE>::type is an integral type whose size (and alignment) is equal to size,
//                       unless size is larger than max_size, then it is equal to max_size.
template<int SIZE, int max_size = sizeof(long)>
struct alignment
{
  enum { log2 = ilog2<SIZE>::value };
  enum { size = 1 << log2 };
  typedef typename size_to_type<size >= max_size ? max_size : size>::integral_t type;
};

#endif // UTILS_ALIGNMENT_H
