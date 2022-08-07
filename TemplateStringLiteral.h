#pragma once

#include "concat_array.h"
#include <algorithm>

namespace utils {

// A type that can be used to pass a string literal as template parameter.
//
// Usage example:
//
// template<TemplateStringLiteral S>
// struct A
// {
//   static constexpr auto str = S.chars;
// };
//
// template<TemplateStringLiteral S>
// constexpr void f(A<S>)
// {
//   std::cout << "The type passed to f() contains \"" << std::string(S.chars.begin(), S.chars.end()) << "\".\n";
// }
//
// int main()
// {
//   constexpr auto ts1 = TemplateStringLiteral{"Hello"};
//   constexpr auto ts2 = TemplateStringLiteral{"World"};               
//  
//   constexpr auto ts12 = Catenate_v<ts1, ts2>;
//  
//   for (char c : ts12.chars)
//     std::cout << c;
//   std::cout << '\n';
//
//   A<ts12> a;
//   f(a);
// }
//
template <std::size_t N>
struct TemplateStringLiteral
{
  std::array<char, N> chars;
  consteval TemplateStringLiteral(std::array<char, N> str) : chars(str) { }
  consteval TemplateStringLiteral(const char (&literal)[N]) { std::copy_n(literal, N, chars.begin()); }
};

template<TemplateStringLiteral S1, TemplateStringLiteral S2>
struct Catenate
{
  static constexpr TemplateStringLiteral value{utils::zconcat_array(S1.chars, S2.chars)};
};

template<TemplateStringLiteral S1, TemplateStringLiteral S2>
inline constexpr auto Catenate_v = Catenate<S1, S2>::value;

} // namespace utils
