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
  std::unordered_map<std::string_view, size_t> m_string_view_to_index;
  std::deque<std::string> m_unique_words;       // This must be a random access container that can be accessed through an index and does not invalidate references.

 private:
  size_t add_extra_word(std::string&& word);

 protected:
  size_t add_new_unique_word(std::string&& word);

 public:
  size_t lookup(std::string_view const& word)
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
  virtual void add_new_data(size_t index, std::string word) = 0;
};

// Usage:
//
// enum elements {
//   foo,
//   bar,
//   baz
// };
//
// struct Data {
//   using index_type elements;
//   Data(index_type, string&&) { }
// };
//
// Dictionary<std::vector<Data>, elements> dictionary;
//
// dictionary.add(foo, "Foo", Data{foo, "Foo", ...});
// dictionary.add(bar, "Bar", Data{bar, "Bar", ...});
// dictionary.add(baz, "Baz", Data{baz, "Baz", ...});
//
// index_type index = dictionary.index("Bar");          // Fast
// ASSERT(index == static_cast<index_type>(bar));
// Data& data = dictionary[bar];                        // Fast
//
// size_t i = dictionary.lookup("unknown");     // Only slow the first time.
// ASSERT(i > (size_t)baz);
// Data& data2 = dictionary[i];                 // Returns a Data constructed with {i, "unknown"}.
//
template<typename DATA_CONTAINER, typename INDEX_TYPE = typename DATA_CONTAINER::index_type>
class Dictionary : public DictionaryBase
{
 public:
  using value_type = typename DATA_CONTAINER::value_type;
  using index_type = INDEX_TYPE;

  static_assert(std::is_constructible_v<index_type, size_t>, "INDEX_TYPE must be constructible from a size_t.");
  static_assert(std::is_constructible_v<value_type, index_type, std::string&&>, "DATA_CONTAINER::value_type must be constructible from (INDEX_TYPE, std::string&&).");

 private:
  DATA_CONTAINER m_data;     // This must be a random access container with an operator[].

 private:
  void add_new_data(size_t index, std::string word) final
  {
    ASSERT(m_data.size() == index);
    m_data.emplace_back(index_type{index}, std::move(word));
  }

 public:
  // Pre-fill the dictionary with pre-defined words.
  // The idea is that if index_type is an enum, then add() should
  // be called for each enumerator in the enum sequentially.
  void add(index_type index, std::string word, value_type data)
  {
    // index must be sequential, starting with 0 and 1 larger every call.
    ASSERT(index_type{m_data.size()} == index);
    m_data.emplace_back(std::move(data));
    add_new_unique_word(std::move(word));
  }

  index_type index(std::string_view const& word) { return static_cast<index_type>(lookup(word)); }

  // Access the stored data by index. This has to be fast too.
  value_type& operator[](index_type index) { return m_data[index]; }
  value_type const& operator[](index_type index) const { return m_data[index]; }
};

} // namespace utils
