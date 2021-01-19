#pragma once

#include "macros.h"
#include <string_view>
#include <string>
#include <unordered_map>
#include <deque>
#include <vector>

namespace utils {

class DictionaryBase
{
 protected:
  std::unordered_map<std::string_view, int> m_string_view_to_index;
  std::deque<std::string> m_unique_words;       // This must be a random access container that can be accessed through an index and does not invalidate references.

 protected:
  int add_new_unique_word(std::string&& word);

 private:
  int add_extra_word(std::string&& word);

 public:
  int index(std::string_view const& word)
  {
    //------------------------------------------------------------------------
    // This is the part that has to be fast.
    auto iter = m_string_view_to_index.find(word);
    if (AI_LIKELY(iter != m_string_view_to_index.end()))
      return iter->second;
    //------------------------------------------------------------------------

    return add_extra_word(std::string(word));
  }

 private:
  virtual void add_new_data(int index, std::string word) = 0;
};

// Usage:
//
// enum elements {
//   foo,
//   bar,
//   baz
// };
//
// Dictionary<Data> dictionary;
//
// dictionary.add(foo, "Foo", Data{foo, "Foo", ...});
// dictionary.add(bar, "Bar", Data{bar, "Bar", ...});
// dictionary.add(baz, "Baz", Data{baz, "Baz", ...});
//
// int i = dictionary.index("Bar");     // Fast
// ASSERT(i == bar);
// Data& data = dictionary[i];          // Fast
//
// i = dictionary.index("unknown");     // Only slow the first time.
// ASSERT(i > (int)baz);
// Data& data2 = dictionary[i]; // Returns a Data constructed with {i, "unknown"}.
//
template<typename DATA>
class Dictionary : public DictionaryBase
{
 public:
  using data_type = DATA;
  using index_type = typename data_type::index_type;
  static_assert(std::is_convertible_v<index_type, int>, "DATA::index_type must be convertible to an int.");
  static_assert(std::is_constructible_v<DATA, int, std::string&&>,
      "DATA must have a constructor that takes a new index and its word. If DATA does not (want to) store this, it can simply ignore those parameters.");

 private:
  std::vector<DATA> m_data;     // This must be a random access container with an operator[].

 private:
  void add_new_data(int index, std::string word) final
  {
    ASSERT(m_data.size() == index);
    m_data.emplace_back(index, std::move(word));
  }

 public:
  // Pre-fill the dictionary with pre-defined words.
  // The idea is that if index_type is an enum, then add() should
  // be called for each enumerator in the enum sequentially.
  void add(index_type index, std::string word, DATA data)
  {
    // index must be sequential, starting with 0 and 1 larger every call.
    ASSERT(m_data.size() == index);
    m_data.emplace_back(std::move(data));
    add_new_unique_word(std::move(word));
  }

  // Access the stored data by index. This has to be fast too.
  DATA& operator[](index_type index) { return m_data[index]; }
  DATA const& operator[](index_type index) const { return m_data[index]; }
};

} // namespace utils
