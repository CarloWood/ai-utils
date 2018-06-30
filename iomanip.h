// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Declaraction of Index, Sticky, Unsticky and Object in namespace utils::iomanip.
//
// Copyright (C) 2017 Carlo Wood.
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

#include <iostream>
#include <cstring>      // std::memset
#include <utility>      // std::move
#include "debug.h"

namespace utils {
namespace iomanip {

// Create a new index for use with ostream iword and pword.
struct Index
{
  int const value;
  Index() : value(std::ios_base::xalloc()) { }          // Requires C++14 to be thread-safe.
  Index(Index const& index) : value(index.value) { }
  operator int() const { return value; }
};

// DataTransfer is used to temporarily store a value before
// it can be written to the ostream (iword or pword).
//
// A value of the index to be used is stored as well (otherwise
// we'd need a virtual function to get it later from the Derived
// class).
//
class DataTransfer
{
  Index m_index;        // A copy of the (static) Index of the final Manipulator type.
  long m_iword;         // The iword that needs to be written to the iword of a stream, if any. Zero if nothing needs to be written.
  void* m_pword;        // The pword that needs to be written to the pword of a stream, if any. Nullptr if nothing needs to be written.

  friend class Sticky;
  DataTransfer(Index const& index, long iword) : m_index(index), m_iword(iword), m_pword(nullptr) { }
  DataTransfer(Index const& index, void* pword) : m_index(index), m_iword(0), m_pword(pword) { }
  DataTransfer(Index const& index, long iword, void* pword) : m_index(index), m_iword(iword), m_pword(pword) { }

 public:
  // Transfer the stored value(s) to a stream.
  void set_on(std::ostream& os) const
  {
    os.iword(m_index) = m_iword;
    os.pword(m_index) = m_pword;
  }

  // Transfer other value(s) to a stream.
  void set_iword_on(std::ostream& os, long iword) const { os.iword(m_index) = iword; }
  void set_pword_on(std::ostream& os, void* pword) const { os.pword(m_index) = pword; }
  void swap_with(std::ostream& os)
  {
    // Swap os.*word() with m_*word.
    long iword = os.iword(m_index);
    void* pword = os.pword(m_index);
    os.iword(m_index) = m_iword;
    os.pword(m_index) = m_pword;
    m_iword = iword;
    m_pword = pword;
  }

  // Get the current iword from a stream (returns 0 if there is none associated with the stream).
  long get_iword_from(std::ostream& os) const { return os.iword(m_index); }

  // Get the current pword from a stream (returns nullptr if there is none associated with the stream).
  void* get_pword_from(std::ostream& os) const { return os.pword(m_index); }
};

// Base class for a sticky IO manipulator.
//
// index must be a reference to a static member object of the most derived class.
// iword / pword are the values that will be associated with the iostream.
//
// Usage:
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.h}
// class MyManipulator : public utils::iomanip::Sticky<2>
// {
//  private:
//   static utils::iomanip::Index s_index;
//
//  public:
//   MyManipulator(long iword_value) : Sticky(s_index, iword_value) { }                    // And/or pass a void*.
//
//   static long get_iword_value(std::ostream& os) { return get_iword_from(os, s_index); } // Or get_pword_from.
// };
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
// //static
// utils::iomanip::Index MyManipulator::s_index;
//
// std::ostream& operator<<(std::ostream& os, Object const& object)
// {
//   if (MyManipulator::get_iword_value(os) == 42L)
//     ...
//   else
//    ...
// }
//
// ...
//      Object object;
//      std::cout << MyManipulator(42L) << object;
// ...
//
// A value of 0 (iword / long) or nullptr (pword / void*) is the default;
// this is returned when the manipulator wasn't used on the stream.
//
class Sticky
{
 protected:
  DataTransfer m_data;

 protected:
  Sticky(Index& index, long iword) : m_data(index, iword) { }
  Sticky(Index& index, void* pword) : m_data(index, pword) { }
  Sticky(Index& index, long iword, void* pword) : m_data(index, iword, pword) { }

  // Actually transfer the data from the Sticky object to an ostream.
  friend std::ostream& operator<<(std::ostream& os, Sticky const& io_manipulator)
  {
    io_manipulator.m_data.set_on(os);
    return os;
  }

 protected:
  void set_on(std::ostream& os) { m_data.set_on(os); }

  static long get_iword_from(std::ostream& os, Index& index) { return os.iword(index.value); }
  static void* get_pword_from(std::ostream& os, Index& index) { return os.pword(index.value); }
};

template<int max_number_of_streams>
class Unsticky;

template<int max_number_of_streams>
struct UnstickyStore
{
  long m_iword[max_number_of_streams];
  void* m_pword[max_number_of_streams];

  void destruct(Unsticky<max_number_of_streams>*);
  void from_to(std::ostream& os, DataTransfer const& data, int stream);
};

// Specialization for max_number_of_streams of 1.
template<>
struct UnstickyStore<1>
{
  void destruct(Unsticky<1>*) { }
  void from_to(std::ostream&, DataTransfer const&, int) { }
};

template<int max_number_of_streams = 1>
class Unsticky : public Sticky
{
 private:
  friend struct UnstickyStore<max_number_of_streams>;
  std::ostream* m_os[max_number_of_streams];
  UnstickyStore<max_number_of_streams> m_store;

 protected:
  Unsticky(Index& index, long iword) : Sticky(index, iword) { std::memset(m_os, 0, sizeof(m_os)); }
  Unsticky(Index& index, void* pword) : Sticky(index, pword) { std::memset(m_os, 0, sizeof(m_os)); }
  Unsticky(Index& index, long iword, void* pword) : Sticky(index, iword, pword) { std::memset(m_os, 0, sizeof(m_os)); }
  ~Unsticky()
  {
    if (max_number_of_streams == 1)
    {
      if (m_os[0]) { this->m_data.set_on(*m_os[0]); }
    }
    else
      m_store.destruct(this);
  }

  friend std::ostream& operator<<(std::ostream& os, Unsticky&& io_manipulator)
  {
    bool first = true;
    int stream = 0;
    while(stream < max_number_of_streams)
    {
      if (!io_manipulator.m_os[stream])
        break;
      if (!(first = io_manipulator.m_os[stream] != &os))
        break;
      ++stream;
    }
    // You can not use this Unsticky IO manipulator on more than max_number_of_streams streams at a time.
    ASSERT(stream < max_number_of_streams);
    io_manipulator.m_os[stream] = &os;

    // Only remember the old values the first time this manipulator object is used.
    // Stream state will be reset to that upon destruction.
    if (max_number_of_streams > 1 && first)
      io_manipulator.m_store.from_to(os, io_manipulator.m_data, stream);
    if (max_number_of_streams == 1 && first)
      io_manipulator.m_data.swap_with(os);
    else
      io_manipulator.m_data.set_on(os);
    return os;
  }
};

template<int max_number_of_streams>
void UnstickyStore<max_number_of_streams>::destruct(Unsticky<max_number_of_streams>* unsticky)
{
  for (int i = 0; i < max_number_of_streams; ++i)
  {
    if (unsticky->m_os[i])
    {
      unsticky->m_data.set_iword_on(*unsticky->m_os[i], m_iword[i]);
      unsticky->m_data.set_pword_on(*unsticky->m_os[i], m_pword[i]);
    }
  }
}

template<int max_number_of_streams>
void UnstickyStore<max_number_of_streams>::from_to(std::ostream& os, DataTransfer const& data, int stream)
{
  m_iword[stream] = data.get_iword_from(os);
  m_pword[stream] = data.get_pword_from(os);
}

template<int max_number_of_streams>
class Object : public Unsticky<max_number_of_streams>
{
 protected:
  Object(Index& index) : Unsticky<max_number_of_streams>(index, this) { }
  static Object* ptr(std::ostream& os, Index& index) { return static_cast<Object*>(Sticky::get_pword_from(os, index)); }

  friend std::ostream& operator<<(std::ostream& os, Unsticky<max_number_of_streams> const& io_manipulator) { return os << std::move(const_cast<Unsticky<max_number_of_streams>&>(io_manipulator)); }
};

} // namespace iomanip
} // namespace utils
