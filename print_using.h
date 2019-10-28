// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Definition of print_using.
//
// Copyright (C) 2019 Carlo Wood.
//
// RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
// Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
//
// This file is part of ai-utils.
//
// ai-utils is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ai-utils is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with ai-utils.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

#include <iosfwd>
#include <functional>

namespace utils {

template<typename T, typename L>
class PrintUsing2
{
 private:
  T m_obj;
  L const m_print_on;

 public:
  PrintUsing2(T obj, L print_on) : m_obj(obj), m_print_on(print_on) { }

  friend std::ostream& operator<<(std::ostream& os, PrintUsing2 const& print_using)
  {
    print_using.m_print_on(os, print_using.m_obj);
    return os;
  }
};

template<typename L>
class PrintUsing1
{
 private:
  L const m_print_on;

 public:
  PrintUsing1(L print_on) : m_print_on(print_on) { }

  friend std::ostream& operator<<(std::ostream& os, PrintUsing1 const& print_using)
  {
    print_using.m_print_on(os);
    return os;
  }
};

template<typename T, typename L>
PrintUsing2<T, L> print_using(T obj, L print_on)
{
  return { obj, print_on };
}

template<typename L>
PrintUsing1<L> print_using(L print_on)
{
  return { print_on };
}

} // namespace utils
