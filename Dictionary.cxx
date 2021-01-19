#include "sys.h"
#include "Dictionary.h"

namespace utils {

int DictionaryBase::add_new_unique_word(std::string&& word)
{
  int index = m_unique_words.size();
  m_unique_words.emplace_back(std::move(word));
  std::string& stored_word = m_unique_words.back();
  m_string_view_to_index.emplace(stored_word, index);
  return index;
}

int DictionaryBase::add_extra_word(std::string&& word)
{
  int index = m_unique_words.size();
  add_new_data(index, word);
  return add_new_unique_word(std::move(word));
}

} // namespace utils
