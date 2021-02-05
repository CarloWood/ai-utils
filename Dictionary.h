#pragma once

#include "macros.h"
#include "debug.h"
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
  size_t add_extra_word(std::string_view const& word);

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

    return add_extra_word(word);
  }

  std::string const& word(int i) const { return m_unique_words[i]; }

 private:
  // This does nothing, unless this is a DictionaryData class.
  virtual void add_new_data(size_t index, std::string word) { }
};

// Usage (using utils::VectorIndex):
//
// enum enum_type {
//   foo,
//   bar,
//   baz
// };
//
// using index_type = utils::VectorIndex<enum_type>;
//
// Dictionary<enum_type, index_type> dictionary;
//
// dictionary.add(foo, "Foo");
// dictionary.add(bar, "Bar");
// dictionary.add(baz, "Baz");
//
// index_type i1 = dictionary.index("Bar");          // Fast
// ASSERT(i1 == static_cast<index_type>(bar));
//
// index_type i2 = dictionary.lookup("unknown");     // Only slow the first time.
// ASSERT(i2 > static_cast<index_type>(baz));
//
template<typename ENUM_TYPE, typename INDEX_TYPE>
class Dictionary : public DictionaryBase
{
 public:
  using enum_type = ENUM_TYPE;
  using index_type = INDEX_TYPE;

  static_assert(std::is_convertible_v<ENUM_TYPE, size_t>, "ENUM_TYPE must be convertible to size_t.");
  static_assert(std::is_constructible_v<index_type, size_t>, "INDEX_TYPE must be constructible from a size_t.");

  // Pre-fill the dictionary with pre-defined words.
  // The idea is that if index_type is an enum, then add() should
  // be called for each enumerator in the enum sequentially.
  void add(ENUM_TYPE index, std::string word)
  {
    // index must be sequential, starting with 0 and 1 larger every call.
    ASSERT(m_unique_words.size() == static_cast<size_t>(index));
    this->add_new_unique_word(std::move(word));
  }

  void add(ENUM_TYPE index, std::string_view&& word)
  {
    // index must be sequential, starting with 0 and 1 larger every call.
    ASSERT(m_unique_words.size() == static_cast<size_t>(index));
    this->add_new_unique_word(std::string{word});
  }

  // Return a unique index for each unique word.
  // Subsequent calls with the same word result in the same return value.
  // If word was passed to add before then the value of index that was passed to add is returned (cast to an index_type).
  index_type index(std::string_view const& word) { return static_cast<index_type>(this->lookup(word)); }
};

// Usage (using utils::Vector):
//
// enum enum_type {
//   foo,
//   bar,
//   baz
// };
//
// using index_type = utils::VectorIndex<enum_type>;
//
// struct Data {
//   Data(index_type, string&&) { }
// };
//
// Dictionary<enum_type, utils::Vector<Data, index_type>> dictionary;
//
// dictionary.add(foo, "Foo", Data{foo, "Foo", ...});
// dictionary.add(bar, "Bar");  // Will construct Data with {static_cast<index_type>(bar), "Bar"};
// dictionary.add(baz, "Baz", Data{baz, "Baz", ...});
//
// index_type i1 = dictionary.index("Bar");             // Fast
// ASSERT(i1 == static_cast<index_type>(bar));
// Data& data1 = dictionary[bar];                       // Fast
//
// index_type i2 = dictionary.index("unknown");         // Only slow the first time.
// ASSERT(i2 > (size_t)baz);
// Data& data2 = dictionary[i2];                // Returns a Data constructed with {i2, "unknown"}.
//
//
// Usage (using std::vector):
//
// using index_type = int;
// Dictionary<enum_type, std:vector<Data>, int> dictionary;
//
template<typename ENUM_TYPE, typename DATA_CONTAINER, typename INDEX_TYPE = typename DATA_CONTAINER::index_type>
class DictionaryData : public Dictionary<ENUM_TYPE, INDEX_TYPE>
{
 public:
  using enum_type = ENUM_TYPE;
  using index_type = INDEX_TYPE;
  using value_type = typename DATA_CONTAINER::value_type;

  static_assert(std::is_constructible_v<value_type, index_type, std::string&&>, "DATA_CONTAINER::value_type must be constructible from (INDEX_TYPE, std::string&&).");

 private:
  DATA_CONTAINER m_data;     // This must be a random access container with an operator[].

 private:
  void add_new_data(size_t index, std::string word) final
  {
    ASSERT(m_data.size() == index);
    m_data.emplace_back(static_cast<index_type>(index), std::move(word));
  }

 public:
  // Pre-fill the dictionary with pre-defined words.
  // The idea is that if index_type is an enum, then add() should
  // be called for each enumerator in the enum sequentially.
  void add(ENUM_TYPE index, std::string word, value_type data)
  {
    Dictionary<ENUM_TYPE, INDEX_TYPE>::add(index, std::move(word));
    m_data.emplace_back(std::move(data));
  }

  // Access the stored data by index. This has to be fast too.
  value_type& operator[](index_type index) { return m_data[index]; }
  value_type const& operator[](index_type index) const { return m_data[index]; }

  // Access the stored data by enum. These are disabled when enum converts to index_type.
  template<typename U1 = enum_type, typename U2 = index_type>
  typename std::enable_if_t<!std::is_convertible_v<U2, U1>, value_type>&
  operator[](enum_type index) { return m_data[static_cast<index_type>(static_cast<size_t>(index))]; }

  template<typename U1 = enum_type, typename U2 = index_type>
  typename std::enable_if_t<!std::is_convertible_v<U2, U1>, value_type> const&
  operator[](enum_type index) const { return m_data[static_cast<index_type>(static_cast<size_t>(index))]; }
};

} // namespace utils
