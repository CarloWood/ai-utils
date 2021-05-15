/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of print_using.
 *
 * @Copyright (C) 2019  Carlo Wood.
 *
 * RSA-1024 0x624ACAD5 1997-01-26                    Sign & Encrypt
 * Fingerprint16 = 32 EC A7 B6 AC DB 65 A6  F6 F6 55 DD 1C DC FF 61
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

//#include "debug.h"
#include "is_pointer_like.h"
#include <iostream>
#include <memory>
#include <functional>

namespace utils {

class PrintUsing1
{
 private:
  void (*m_print_on)(std::ostream&);

 public:
  PrintUsing1(void (*print_on)(std::ostream&)) : m_print_on(print_on) { }

  friend std::ostream& operator<<(std::ostream& os, PrintUsing1 const& print_using)
  {
    print_using.m_print_on(os);
    return os;
  }
};

PrintUsing1 print_using(void (*print_on)(std::ostream&));

template<typename> struct member_function_traits
{
  using instance_type = void;
};

template<typename Return, typename Object, typename... Args>
struct member_function_traits<Return (Object::*)(Args...)>
{
  using instance_type = Object;
};

template<typename Return, typename Object, typename... Args>
struct member_function_traits<Return (Object::*)(Args...) const>
{
  using instance_type = Object;
};

template<typename T, typename print_on_type>
class PrintUsing2
{
 private:
  T m_obj;
  print_on_type m_print_on;

 public:
  PrintUsing2(T obj, print_on_type print_on) : m_obj(obj), m_print_on(print_on) { }

  friend std::ostream& operator<<(std::ostream& os, PrintUsing2 print_using)
  {
    //Dout(dc::notice, "Using " << type_info_of<PrintUsing2>().demangled_name());
    if constexpr (utils::is_pointer_like_dereferencable_v<T>)
    {
      if constexpr (((std::is_member_function_pointer_v<print_on_type> &&
                        std::is_same_v<std::decay_t<decltype(*std::declval<T>())>, typename member_function_traits<print_on_type>::instance_type>) ||
                      !std::is_member_function_pointer_v<print_on_type>))
      {
        os << '*';
        if constexpr (std::is_member_function_pointer_v<print_on_type>)
          ((*print_using.m_obj).*print_using.m_print_on)(os);
        else
          print_using.m_print_on(os, *print_using.m_obj);
      }
      // This part...
      else if constexpr (std::is_member_function_pointer_v<print_on_type>)
        (print_using.m_obj.*print_using.m_print_on)(os);
      else
        print_using.m_print_on(os, print_using.m_obj);
    }
    // Is repeated here...
    else if constexpr (std::is_member_function_pointer_v<print_on_type>)
      (print_using.m_obj.*print_using.m_print_on)(os);
    else
      print_using.m_print_on(os, print_using.m_obj);

    return os;
  }
};

template<typename T> using PrintUsing2_by_value = PrintUsing2<T, void(*)(std::ostream&, T)>;
template<typename T> using PrintUsing2_by_const_reference = PrintUsing2<T const&, void(*)(std::ostream&, T const&)>;
template<typename T> using PrintUsing2_by_const_member_function = PrintUsing2<T const&, void(T::*)(std::ostream&) const>;
template<typename T> using PrintUsing2_by_std_function = PrintUsing2<T const&, std::function<void(std::ostream&, T const&)>>;

template<typename T>
PrintUsing2_by_value<T> print_using(T obj, void (*print_on)(std::ostream&, T))
{
  return { obj, print_on };
}

template<typename T>
PrintUsing2_by_const_reference<T> print_using(T const& obj, void (*print_on)(std::ostream&, T const&))
{
  return { obj, print_on };
}

template<typename T>
PrintUsing2_by_const_member_function<T> print_using(T const& obj, void (T::*print_on)(std::ostream&) const)
{
  return { obj, print_on };
}

template<typename T>
struct PrintUsingIdentity {
  using type = T;
};

template<typename T>
PrintUsing2_by_std_function<T> print_using(T const& obj, std::function<void(std::ostream&, typename PrintUsingIdentity<T>::type const&)> print_on)
{
  return { obj, print_on };
}

template<typename T> using DereferencedType_t = std::remove_reference_t<decltype(*std::declval<T>())>;
template<typename T, std::enable_if_t<utils::is_pointer_like_dereferencable_v<T>, int> = 0> using PrintUsingPtr_by_value = PrintUsing2<T, void(*)(std::ostream&, DereferencedType_t<T>)>;
template<typename T, std::enable_if_t<utils::is_pointer_like_dereferencable_v<T>, int> = 0> using PrintUsingPtr_by_const_reference = PrintUsing2<T const&, void(*)(std::ostream&, DereferencedType_t<T> const&)>;
template<typename T, std::enable_if_t<utils::is_pointer_like_dereferencable_v<T>, int> = 0> using PrintUsingPtr_by_const_member_function = PrintUsing2<T const&, void(DereferencedType_t<T>::*)(std::ostream&) const>;
template<typename T, std::enable_if_t<utils::is_pointer_like_dereferencable_v<T>, int> = 0> using PrintUsingPtr_by_std_function = PrintUsing2<T const&, std::function<void(std::ostream&, DereferencedType_t<T> const&)>>;

template<typename T>
PrintUsingPtr_by_value<T> print_using(T obj, void (*print_on)(std::ostream&, DereferencedType_t<T>))
{
  return { obj, print_on };
}

template<typename T>
PrintUsingPtr_by_const_reference<T> print_using(T const& obj, void (*print_on)(std::ostream&, DereferencedType_t<T> const&))
{
  return { obj, print_on };
}

template<typename T>
PrintUsingPtr_by_const_member_function<T> print_using(T const& obj, void (DereferencedType_t<T>::*print_on)(std::ostream&) const)
{
  return { obj, print_on };
}

template<typename T>
PrintUsingPtr_by_std_function<T> print_using(T const& obj, std::function<void(std::ostream&, DereferencedType_t<T> const&)> print_on)
{
  return { obj, print_on };
}

} // namespace utils
