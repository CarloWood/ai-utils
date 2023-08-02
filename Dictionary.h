#pragma once

#include "macros.h"
#include "debug.h"
#include <string_view>
#include <string>
#include <unordered_map>
#include <deque>
#include <vector>
#include <exception>

namespace utils {

class DictionaryBase
{
 protected:
  std::unordered_map<std::string_view, size_t> m_string_view_to_index;
  std::deque<std::string> m_unique_words;       // This must be a random access container that can be accessed through an index and does not invalidate references.

 protected:
  size_t add_new_unique_word(std::string&& word);

 public:
  // This should be called when lookup throws.
  size_t add_extra_word(std::string_view const& word);

  struct NonExistingWord : std::exception { };
  size_t lookup(std::string_view const& word) const
  {
    //------------------------------------------------------------------------
    // This is the part that has to be fast.
    auto iter = m_string_view_to_index.find(word);
    if (AI_LIKELY(iter != m_string_view_to_index.end()))
      return iter->second;
    //------------------------------------------------------------------------

    throw NonExistingWord{};
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
// index_type i1 = dictionary.index("Bar");          // Fast (throws if "Bar" doesn't exist!)
// ASSERT(i1 == static_cast<index_type>(bar));
//
// std::string_view word("unknown");
// index_type i2;
// try
// {
//   i2 = dictionary.lookup(word);                   // Throws when word is unknown.
// }
// catch (NonExistingWord const&)
// {
//   i2 = dictionary.add_extra_word(word);
// }
// ASSERT(i2 > static_cast<index_type>(baz));
//
//
// Usage (using threadsafe):
//
// If the dictionary can be used by multiple threads, for example because it is
// a static member of a template class, then it should be wrapped by threadsafe
// using a read/write lock:
//
// template<typename T>
// struct Example {
//   using dictionary_type = threadsafe::Unlocked<utils::Dictionary<typename T::enum_type, index_type>, threadsafe::policy::ReadWrite<AIReadWriteMutex>>;
//   static dictionary_type s_dictionary;
//   ...
// };
//
// // Instantiate the dictionary.
// template<typename T>
// typename Example<T>::dictionary_type Example<T>::s_dictionary;
//
// And then initialization can be done, for example, like this (using magic_enum to convert the enumerator names to a string):
//
// template<typename T>
// Example<T>::Example()
// {
//   // Only initialize the (static) dictionary once!
//   // See https://stackoverflow.com/a/36596693/1487069
//   static bool once = [](){
//     typename dictionary_type::wat dictionary_w(s_dictionary);
//     for (int i = 0; i < magic_enum::enum_count<typename T::enum_type>(); ++i)
//     {
//       typename T::enum_type e = static_cast<typename T::enum_type>(i);
//       std::string_view e_name = magic_enum::enum_name(e);
//       dictionary_w->add(e, std::move(e_name));
//     }
//     return true;
//   } ();
// }

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
  // If word was not added yet, then this function throws NonExistingWord and word should be passed to add_extra_word.
  index_type index(std::string_view const& word) const { return static_cast<index_type>(this->lookup(word)); }
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
// index_type i1 = dictionary.index("Bar");     // Fast (would throw if "Bar: wasn't added above).
// ASSERT(i1 == static_cast<index_type>(bar));
// Data& data1 = dictionary[bar];               // Fast
//
// std::string_view word("unknown");
// try
// {
//   i2 = dictionary.index(word);               // Throws NonExistingWord.
// }
// catch (NonExistingWord const&)
// {
//   i2 = dictionary.add_extra_word(word);
// }
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
