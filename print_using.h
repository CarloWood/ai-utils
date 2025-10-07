/**
 * ai-utils -- C++ Core utilities
 *
 * @file
 * @brief Definition of print_using.
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

#include "is_pointer_like.h"
#include <iostream>
#include <memory>
#include <functional>
#include "debug.h"

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
      if constexpr (
          (std::is_member_function_pointer_v<print_on_type> &&
           // If m_print_on is a member function pointer, then only dereference m_obj if the result is the class type of that member function.
           std::is_same_v<std::decay_t<decltype(*std::declval<T>())>, typename member_function_traits<print_on_type>::instance_type>) ||
          (!std::is_member_function_pointer_v<print_on_type> &&
           // If m_print_on(os, m_obj) compiles, then do not attempt to dereference m_obj just because we can.
           !requires(std::ostream& os_p, print_on_type print_on_p, T obj_p) { print_on_p(os_p, obj_p); }))
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

// Concept for classes that are invokable passing (std::ostream&, T const&).
// For example,
//
// struct MyPrinter {
//   void operator()(std::ostream&, T const&);
// };
template<typename Printer, typename T>
concept ConceptPrinter =
  std::invocable<Printer&, std::ostream&, T const&> &&
  std::same_as<std::invoke_result_t<Printer&, std::ostream&, T const&>, void>;

template<typename T, ConceptPrinter<T> print_on_type>
PrintUsing2<T const&, print_on_type const&> print_using(T const& obj, print_on_type const& printer)
{
  return { obj, printer };
}

} // namespace utils
