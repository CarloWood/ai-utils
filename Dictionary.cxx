#include "sys.h"
#include "Dictionary.h"

namespace utils {

size_t DictionaryBase::add_new_unique_word(std::string&& word)
{
  size_t index = m_unique_words.size();
  m_unique_words.emplace_back(std::move(word));
  std::string& stored_word = m_unique_words.back();
  m_string_view_to_index.emplace(stored_word, index);
  return index;
}

size_t DictionaryBase::add_extra_word(std::string_view const& word)
{
  std::string extra_word(word);
  add_new_data(m_unique_words.size(), extra_word);
  return add_new_unique_word(std::move(extra_word));
}

} // namespace utils
