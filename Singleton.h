// ai-utils -- C++ Core utilities
//
//! @file
//! @brief Declaraction of class Singleton.
//
// Copyright (C) 2014, 2016  Carlo Wood.
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

//! @class Singleton
//! @brief Template wrapper to turn custom classes into a true singleton.
//
// Usage:
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.h}
// class MySingleton : public Singleton<MySingleton> {
//   friend_Instance;
// private:    // IMPORTANT! DO NOT FORGET THIS `private:' and the three declarations below!
//             // Without it, this is not a real singleton!
//   MySingleton() { }
//   ~MySingleton() { }
//   MySingleton(MySingleton const&) = delete;
//
// public:
//   // ...
// };
//
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cpp}
// namespace {
// SingletonInstance<MySingleton> dummy __attribute__ ((__unused__));
// } // namespace
//
// int main(int argc, char* argv[])
// {
//   Debug(NAMESPACE_DEBUG::init());
//
//   // ...
//   // Use `MySingleton::instance()' here.
//
//   // HOWEVER: Use `MySingleton::instantiate()' from constructors of other
//   // static/global objects that could be called before main() is!
// }
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include "Global.h"		// Must be included before Singleton.h

#ifndef UTILS_LIBCW_SINGLETON_H
#define UTILS_LIBCW_SINGLETON_H

#include "debug.h"

/// @cond Doxygen_Suppress
namespace utils {
  namespace _internal_ {
    int const singleton = -1;
  }	// namespace _internal_
}	// namespace libcw
/// @endcond

#ifdef DEBUGGLOBAL
// Specialization for printing the type name
template<class TYPE, class CONVERTER>
std::ostream& operator<<(std::ostream& os, typename utils::_internal_::GlobalTypeName<TYPE, utils::_internal_::singleton, CONVERTER> const&)
{
  os << "Singleton<" << libcwd::type_info_of<TYPE>().demangled_name() << '>';
  return os;
}

template<class TYPE, class CONVERTER>
std::ostream& operator<<(std::ostream& os, typename utils::_internal_::GlobalInstanceTypeName<TYPE, utils::_internal_::singleton, CONVERTER> const&)
{
  os << "SingletonInstance<" << libcwd::type_info_of<TYPE>().demangled_name() << '>';
  return os;
}
#endif

template<class FINAL, class CHILD = FINAL>
class Singleton
{
protected:
  using final_type = FINAL;
  using Instance = utils::_internal_::Instance<FINAL, utils::_internal_::singleton, GlobalConverterVoid>;
  ~Singleton() { }
#ifndef DEBUGGLOBAL
  Singleton() { }
#else
  Singleton();
#endif
protected:
  Singleton(Singleton const&);
  Singleton& operator=(Singleton const& s);
public:
#ifndef DEBUGGLOBAL
  static inline FINAL& instantiate() { return Global<FINAL, utils::_internal_::singleton, GlobalConverterVoid>::instantiate(); }
#else
  static FINAL& instantiate();
#endif
  static inline FINAL& instance() { return Global<FINAL, utils::_internal_::singleton, GlobalConverterVoid>::instance(); }
};

#define friend_Instance friend class utils::_internal_::Instance<final_type, utils::_internal_::singleton, GlobalConverterVoid>; \
                        friend class utils::_internal_::GlobalBase<final_type, utils::_internal_::singleton>::InstanceDummy

template<class FINAL> using SingletonInstance = Global<FINAL, utils::_internal_::singleton, GlobalConverterVoid>;

//-------------------------------------------------------------------------
// Definitions

#ifdef DEBUGGLOBAL
template<class FINAL, class CHILD>
FINAL& Singleton<FINAL, CHILD>::instantiate()
{
  FINAL& ref = Global<FINAL, utils::_internal_::singleton, GlobalConverterVoid>::instantiate();
  Global<FINAL, utils::_internal_::singleton, GlobalConverterVoid>::set_instantiate_return_address0(__builtin_return_address(0));
  Global<FINAL, utils::_internal_::singleton, GlobalConverterVoid>::set_instantiate_return_address1(__builtin_return_address(1));
  return ref;
}
#endif

template<class FINAL, class CHILD>
Singleton<FINAL, CHILD>::Singleton(Singleton const&)
{
  DoutFatal( dc::core, "Make the copy constructor of your singletons private!" );
}

template<class FINAL, class CHILD>
Singleton<FINAL, CHILD>& Singleton<FINAL, CHILD>::operator=(Singleton const& s)
{
  if (this != &s)
    DoutFatal( dc::core, "Assignment of singleton?!" );
}

#ifdef DEBUGGLOBAL
template<class FINAL, class CHILD>
Singleton<FINAL, CHILD>::Singleton()
{
  if (!Global<FINAL, utils::_internal_::singleton, GlobalConverterVoid>::beingInstantiatedRightNow_())
  {
    std::string s(libcwd::type_info_of(*this).demangled_name());
    DoutFatal( dc::core, "Make the constructor of `" << s.substr(14, s.size() - 15) << "' private!" );
  }
}
#endif

#endif // UTILS_LIBCW_SINGLETON_H
