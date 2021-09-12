#pragma once

#include <string_view>
#include <array>

namespace utils {

// Call found_token(sv_token) for every token.
// Empty tokens exist too; found_token is called
// at least once: as often as there are delimiters plus one.
template<typename L>
void split(std::string_view str, char delim, L found_token)
{
  auto first = str.begin();
  auto const end = str.end();
  for (auto iter = first;; ++iter)
  {
    if (iter == end || *iter == delim)
    {
      found_token(std::string_view{first, static_cast<size_t>(iter - first)});
      if (iter == end)
        break;
      first = iter + 1;
    }
  }
}

template<size_t N>
void splitN(std::string_view str, char delim, std::array<std::string_view, N>& output)
{
  int i = 0;
  auto first = str.begin();
  auto const end = str.end();
  for (auto iter = first;; ++iter)
  {
    if (iter == end || *iter == delim)
    {
      output[i++] = std::string_view{first, static_cast<size_t>(iter - first)};
      if (iter == end)
        break;
      if (i == N)
        THROW_ALERT("Too many separator characters ('[DELIM]') in \"[STR]\" (exactly [N] [VERB] required)",
            AIArgs("[DELIM]", delim)("[STR]", str)("[N]", N - 1)("[VERB]", N == 2 ? "is" : "are"));
      first = iter + 1;
    }
  }
  if (i != N)
    THROW_ALERT("Not enough separator characters ('[DELIM]') in \"[STR]\" (exactly [N] [VERB] required)",
        AIArgs("[DELIM]", delim)("[STR]", str)("[N]", N - 1)("[VERB]", N == 2 ? "is" : "are"));
}

} // namespace utils
