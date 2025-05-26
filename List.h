#pragma once

#include <memory>
#include <iterator>
#include <algorithm>
#include <limits>
#include <bit>
#include <compare>
#include "utils/macros.h"
#include "debug.h"

// utils::List<T> is doubly-linked list like std::list<T>.
//
// Like std::list, the iterator is just a wrapper around a Node* (sizeof(iterator) == sizeof(void*)).
//
// utils::List has the following extra features:
//
//    * iterator is derived from const_iterator; making the conversion from iterator to const_iterator a non-op
//      and allowing a static_cast in the opposite direction.
//    * const_iterator (and therefore iterator) have the member functions:
//        bool is_begin();
//        bool is_end();
//      which return if the current iterator is pointing to the begin() or end() of a list, without the
//      need to have access to the list itself!
//      For example, instead of requiring `iter != my_list.end()` it is possible to do `!iter.is_end()`
//      (which is just a single bitwise AND).
//    * the value_type gives access to the previous and next elements, also without the need to have access
//      to the underlying list.
//      For example,
//        list_type::value_type first_element = my_list.front();
//        list_type::iterator second_element_iter = first_element.next();    // Doesn't need my_list.
//        if (!second_element_iter.is_end())
//        {
//          list_type::value_type second_element = *second_element_iter;
//
// The latter feature means, however, that value_type != T.
// This list has:
//
//   template<typename T>
//   class List {
//    public:
//     class value_type : protected Node {
//       T data_;
//
// where Node is defined as
//
//   struct Node {
//     Node* prev_;
//     Node* next_;
//   };
//
// Note that value_type can be constructed like T and automatically converts to T; so it can be treated
// as-if it is of type T - it is just larger.
//
// A std::list uses the same amount of memory, it just defines value_type to be T:
//
//   template<typename T>
//   class list {
//    public:
//     using value_type = T;
//    private:
//     class Data : public Node {
//       T value_;

#define DEBUG_MERGE_SORT 0

#if DEBUG_MERGE_SORT
#include "utils/print_using.h"
#endif

namespace utils {

template<typename T>
class ListBase
{
 public:
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

 public:
  class const_iterator;
  class iterator;

  struct Node
  {
    Node* prev_;
    Node* next_;

    bool is_tainted() const { return reinterpret_cast<uintptr_t>(prev_) & 1; }
    uintptr_t get_taint() const { return reinterpret_cast<uintptr_t>(prev_) & 1; }
  };

 protected:
  Node base_node_{taint(&base_node_), &base_node_};     // The base node of the list. Contains pointers to the first and last node.
  size_type size_{0};                                   // A count of the number of elements currently in the list.

 protected:
  static Node* taint(Node* ptr) { return reinterpret_cast<Node*>(reinterpret_cast<uintptr_t>(ptr) | 1); }
  static Node* untaint(Node* ptr) { return reinterpret_cast<Node*>(reinterpret_cast<uintptr_t>(ptr) & ~uintptr_t{1}); }
  [[gnu::always_inline]] static Node* extract_taint(Node* ptr, uintptr_t& taint_out)
  {
    uintptr_t integral = reinterpret_cast<uintptr_t>(ptr);
    taint_out = integral & 1;
    return reinterpret_cast<Node*>(integral & ~uintptr_t{1});
  }
  // Assign the result to the argument that extract_taint was called with, passing the return of that as second parameter.
  [[gnu::always_inline]] static Node* assign_with_taint(Node* new_ptr, uintptr_t extracted_taint)
  {
    return reinterpret_cast<Node*>(reinterpret_cast<uintptr_t>(new_ptr) | extracted_taint);
  }

 public:
  class value_type : protected Node
  {
    // Need access to Node.
    friend class const_iterator;
    friend class iterator;
    friend class ListBase;

   private:
    T data_;

   public:
    template<typename... Args>
    value_type(Args&&... args) : data_(std::forward<Args>(args)...) { }

    inline iterator prev();
    inline const_iterator prev() const;

    inline iterator next();
    inline const_iterator next() const;

    operator T&() { return data_; }
    operator T const&() const { return data_; }
  };

  using reference = value_type&;
  using const_reference = value_type const&;

  // Iterator types.

  class const_iterator
  {
   public:
    // Iterator type aliases.
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = ListBase::value_type;
    using difference_type = ListBase::difference_type;
    using pointer = value_type const*;
    using reference = value_type const&;

   private:
    friend class iterator;
    friend class ListBase;
    Node* ptr_;

   public:
    const_iterator() = default;
    explicit const_iterator(Node* ptr) : ptr_(ptr) { }
    // Construct an `end()` iterator.
    const_iterator(ListBase const& list) : ptr_(const_cast<Node*>(&list.base_node_)) { }

    bool is_end() const { return ptr_->is_tainted(); }
    bool is_begin() const { return !is_end() && ptr_->prev_->is_tainted(); }

    pointer operator->() const { ASSERT(!is_end()); return static_cast<pointer>(ptr_); }
    reference operator*() const { ASSERT(!is_end()); return static_cast<reference>(*ptr_); }

    const_iterator& operator++()
    {
      ptr_ = ptr_->next_;
      return *this;
    }

    const_iterator operator++(int)
    {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    const_iterator& operator--()
    {
      ptr_ = ListBase<T>::untaint(ptr_->prev_);
      return *this;
    }

    const_iterator operator--(int)
    {
      const_iterator tmp = *this;
      --(*this);
      return tmp;
    }

    friend bool operator==(const_iterator const& rhs, const_iterator const& lhs) { return rhs.ptr_ == lhs.ptr_; }
    friend bool operator!=(const_iterator const& rhs, const_iterator const& lhs) { return rhs.ptr_ != lhs.ptr_; }
  };

  class iterator : public const_iterator
  {
    using base = const_iterator;

   public:
    using pointer = value_type*;
    using reference = value_type&;

   public:
    using const_iterator::const_iterator;

    pointer operator->() const { return static_cast<pointer>(base::ptr_); }
    reference operator*() const { return static_cast<reference>(*base::ptr_); }

    iterator& operator++() { return static_cast<iterator&>(++static_cast<const_iterator&>(*this)); }
    iterator operator++(int) { iterator result{*this}; operator++(); return result; }
    iterator& operator--() { return static_cast<iterator&>(--static_cast<const_iterator&>(*this)); }
    iterator operator--(int) { iterator result{*this}; operator--(); return result; }
  };

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

 protected:
  // Cast back and forth between value_type* and Node*.
  static Node* node_cast(value_type* value) { return static_cast<Node*>(value); }
  static value_type* value_type_cast(Node* node) { return static_cast<value_type*>(node); }

  // Return the number of elements in the range [first, last).
  static size_type count(const_iterator first, const_iterator last)
  {
    size_type result = 0;
    for (Node* node = first.ptr_; node != last.ptr_; node = node->next_)
      ++result;
    return result;
  }

  // Return true if pos == end().
  bool is_end(const_iterator pos) const { return pos.ptr_ == &base_node_; }

  // Add new_node to the end. new_node must have been allocated with List<T, Allocator>::create_node.
  void push_back_node(Node* new_node)
  {
    new_node->next_ = &base_node_;
    new_node->prev_ = untaint(base_node_.prev_);

    new_node->prev_->next_ = new_node;
    base_node_.prev_ = taint(new_node);

    ++size_;
  }

  // Append new_node after `after`.
  // This function may be called repeatedly where each subsequent call must use the previous new_node as after, appending a whole chain.
  // After the last call, one must call link(last_new_node, next_existing_node), where next_existing_node is not allowed to be end,
  // otherwise call link(last_new_node, possibly_end, extracted_taint).
  // size_ is not updated and must be updated separately.
  [[gnu::always_inline]] void link_node(Node* after, Node* new_node)
  {
    after->next_ = new_node;
    new_node->prev_ = after;            // new_node != end, therefore prev_ is not tainted.
  }

  // Same but with optional taint. In this case new_node is allowed to be end (provided the correct taint is supplied).
  // size_ is not updated and must be updated separately.
  [[gnu::always_inline]] void link_node(Node* after, Node* new_node, uintptr_t extracted_taint)
  {
    after->next_ = new_node;
    new_node->prev_ = assign_with_taint(after, extracted_taint);
  }

  // Same, but insert pre-allocated new_node before pos.
  // pos is not allowed to be end(), use push_back_node in that case.
  void insert_node(const_iterator pos, Node* new_node)
  {
    link_node(pos.ptr_->prev_, new_node);       // Connect pos.ptr_->prev_ ⇄ new_node
    link_node(new_node, pos.ptr_);              // Connect                   new_node ⇄ pos
    ++size_;
  }

  // Does NOT update size_!
  // The taint of base_node_.prev_ must be fixed if the list is empty after this call.
  Node* extract_front()
  {
    // From:
    //
    //   | base_node_      | begin           | second_element  |
    //                       |                 |
    //   ____________________v_________________v________________
    //   |        next_ -->|        next_ -->|                 |
    //   |                 |<-- prev_        |<-- prev_        |
    //
    // To:
    //
    //   | base_node_      | second_element  |
    //                       |
    //   ____________________v________________
    //   |        next_ -->|                 |
    //   |                 |<-- prev_        |
    //
    Node* begin = base_node_.next_;
    base_node_.next_ = begin->next_;
    base_node_.next_->prev_ = &base_node_;
    return begin;
  }

  Node* extract_back()
  {
    // From:
    //
    //   | second_last     | last            | base_node_      |
    //     |                 |
    //   __v_________________v__________________________________
    //   |        next_ -->|        next_ -->|                 |
    //   |                 |<-- prev_        |<-- prev_        |
    //
    // To:
    //
    //   | second_last     | base_node_      |
    //     |
    //   __v__________________________________
    //   |        next_ -->|                 |
    //   |                 |<-- prev_        |
    //
    Node* last = untaint(base_node_.prev_);
    last->prev_->next_ = &base_node_;
    base_node_.prev_ = taint(last->prev_);
    return last;
  }

  Node* extract_node_before(const_iterator position)
  {
    // position is not allowed to be end(), use extract_back in that case.
    // position is also not allowed to be begin()+1, use extract_front in that case.

    // From:
    //
    //   | pos_minus_two   | extracted_node  | pos             |
    //     |                 |                 |
    //   __v_________________v_________________v________________
    //   |        next_ -->|        next_ -->|                 |
    //   |                 |<-- prev_        |<-- prev_        |
    //
    // To:
    //
    //   | pos_minus_two   | pos             |
    //     |                 |
    //   __v_________________v________________
    //   |        next_ -->|                 |
    //   |                 |<-- prev_        |
    //
    Node* pos = position.ptr_;
    Node* extracted_node = pos->prev_;
    link_node(extracted_node->prev_, pos);      // Connect pos_minus_two ⇄ pos.
    return extracted_node;
  }

  // Remove the range [first, last) from the list, returns the last element removed.
  // The range is not allowed to be empty.
  Node* extract_range(const_iterator first, const_iterator last)
  {
    // From:
    //                                                                             last
    //   | first_minus_one | first           |                 | last_minus_one  | possibly_end    |
    //     |                 |                                   |                 |
    //   __v_________________v_____________________........______v_________________v________________
    //   |        next_ -->|        next_ -->|              -->|        next_ -->|                 |
    //   |                 |<-- prev_        |<--              |<-- prev_        |<--   prev_      |
    //
    // To:
    //                       last
    //   | first_minus_one | possibly_end    |
    //     |                 |
    //   __v_________________v________________
    //   |        next_ -->|                 |
    //   |                 |<--   prev_      |
    //
    //   | first           |                 | last_minus_one  |
    //     |                                   |
    //   __v_____________________........______v________________
    //   |        next_ -->|              -->|        next_ -->|
    //   |<-- prev_        |<--              |<-- prev_        |
    //
    // Where last_minus_one is returned.
    //
    Node* possibly_end = last.ptr_;             // last can be end().
    uintptr_t extracted_taint;
    Node* last_minus_one = extract_taint(possibly_end->prev_, extracted_taint);
    Node* first_minus_one = first.ptr_->prev_;  // This is not tainted because the range is not empty and thus first can not be end().
    link_node(first_minus_one, possibly_end, extracted_taint);
    return last_minus_one;
  }

  // Find the element following the first count elements.
  // Only call this when size_ > count (i.e. size_ > 0 in all cases).
  const_iterator element_after(size_type count)
  {
    Node* result;
    if (count < size_ / 2)
    {
      // Point to the first element in the list.
      result = base_node_.next_;
      // Skip count elements.
      for (int i = 0; i < count; ++i)
        result = result->next_;
    }
    else
    {
      // Point to the last element in the list.
      result = untaint(base_node_.prev_);
      // Skip (size_ - 1) - count elements.
      //
      // For example,
      // A B C D E    size_ = 5
      //         ^
      //         |
      //        result
      // A B C D E    count = 3
      //       ^
      //       |
      //      result
      // result had to be decremented size_ - 1 - count = 1 times.
      for (int i = size_ - 1; i > count; --i)
        result = result->prev_;
    }

    return const_iterator{result};
  }

 public:
  // Capacity
  [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
  size_type size() const noexcept { return size_; }
  size_type max_size() const noexcept { return std::numeric_limits<difference_type>::max(); }

  void pop_front()
  {
    extract_front();
    if (AI_UNLIKELY(--size_ == 0))
      base_node_.prev_ = taint(&base_node_);
  }

  void pop_back()
  {
    extract_back();
    --size_;
  }

  reference front() { return *static_cast<value_type*>(base_node_.next_); }
  const_reference front() const { *static_cast<value_type const*>(base_node_.next_); }

  reference back() { return *static_cast<value_type*>(untaint(base_node_.prev_)); }
  const_reference back() const { return *static_cast<value_type const*>(untaint(base_node_.prev_)); }

  // Iterator functions
  iterator begin() noexcept { return iterator{base_node_.next_}; }
  const_iterator begin() const noexcept { return const_iterator{base_node_.next_}; }
  const_iterator cbegin() const noexcept { return const_iterator{base_node_.next_}; }

  iterator end() noexcept { return {*this}; }
  const_iterator end() const noexcept { return {*this}; }
  const_iterator cend() const noexcept { return {*this}; }

  reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
  const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
  const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator{cend()}; }

  reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
  const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
  const_reverse_iterator crend() const noexcept { return const_reverse_iterator{cbegin()}; }

  // Insert [first, last) from other before pos. The number of elements in the range must be passed in range_size.
  void splice(const_iterator pos, ListBase& other, const_iterator first, const_iterator last, size_type const range_size)
  {
    if (AI_UNLIKELY(range_size == 0))
      return;

    // Extract the elements of other.
    Node* last_node = other.extract_range(first, last);         // last_node will point to the element *before* last.
    other.size_ -= range_size;

    // From:
    //
    //                       pos
    //   | pos_minus_one   | possibly_end    |
    //     |                 |
    //   __v_________________v________________
    //            next_ -->|
    //                     |<-- prev_
    //
    // To:
    //                                                                             pos
    //   | pos_minus_one   | first_node      |                 | last_node       | possibly_end    |
    //     |                 |                                   |                 |
    //   __v_________________v_____________________........______v_________________v________________
    //   |        next_ -->|        next_ -->|              -->|        next_ -->|                 |
    //   |                 |<-- prev_        |<--              |<-- prev_        |<--   prev_      |

    Node* first_node = first.ptr_;
    Node* possibly_end = pos.ptr_;                // pos can be end().
    uintptr_t extracted_taint;
    Node* pos_minus_one = extract_taint(possibly_end->prev_, extracted_taint);
    link_node(pos_minus_one, first_node);
    link_node(last_node, possibly_end, extracted_taint);
    size_ += range_size;
  }
  void splice(const_iterator pos, ListBase& other);
  void splice(const_iterator pos, ListBase&& other) { splice(pos, other); }
  void splice(const_iterator pos, ListBase& other, const_iterator it) { splice(pos, other, it, std::next(it), 1); }
  void splice(const_iterator pos, ListBase&& other, const_iterator it) { splice(pos, other, it); }
  void splice(const_iterator pos, ListBase& other, const_iterator first, const_iterator last) { splice(pos, other, first, last, count(first, last)); }
  void splice(const_iterator pos, ListBase&& other, const_iterator first, const_iterator last) { splice(pos, other, first, last); }

  template<typename Compare>
  void merge(ListBase& other, Compare comp);
  void merge(ListBase& other) { merge(other, std::less<T>()); }
  void merge(ListBase&& other) { merge(other); }
  template<typename Compare>
  void merge(ListBase&& other, Compare comp) { merge(other, comp); }

  void reverse() noexcept;

  template<typename Compare>
  requires std::predicate<Compare&, T const&, T const&>
  void sort(Compare comp);

  void sort() { sort(std::less<>{}); }

#if DEBUG_MERGE_SORT
  struct WrapIterator { iterator it_; };
  static void dump_single_linked_list_on(std::ostream& os, WrapIterator wrap);
#endif
};

template<typename T, typename Allocator = std::allocator<typename ListBase<T>::value_type>>
class List : public ListBase<T>
{
 public:
  using allocator_type = Allocator;
  using value_type = ListBase<T>::value_type;
  using size_type = ListBase<T>::size_type;
  using difference_type = ListBase<T>::difference_type;
  using reference = ListBase<T>::reference;
  using const_reference = ListBase<T>::const_reference;
  using pointer = std::allocator_traits<Allocator>::pointer;
  using const_pointer = std::allocator_traits<Allocator>::const_pointer;
  using iterator = ListBase<T>::iterator;
  using const_iterator = ListBase<T>::const_iterator;
  using reverse_iterator = ListBase<T>::reverse_iterator;
  using const_reverse_iterator = ListBase<T>::const_reverse_iterator;

  using node_type = ListBase<T>::Node;

 private:
  Allocator alloc_{};           // The allocator of this list.

 private:
  template<typename... Args>
  node_type* create_node(Args&&... args);

 public:
  // Constructors.
  List() = default;
  explicit List(Allocator const& alloc) : alloc_(alloc) { }
  List(List const& other);
  List(List const& other, Allocator const& alloc);
  List(List&& other);
  List(List&& other, Allocator const& alloc);
  List(size_type count, T const& value, Allocator const& alloc = Allocator());
  explicit List(size_type count, Allocator const& alloc = Allocator());

  template<typename InputIt>
  requires (std::convertible_to<std::iter_reference_t<InputIt>, T>)
  List(InputIt first, InputIt last, Allocator const& alloc = Allocator());

  List(std::initializer_list<T> init, Allocator const& alloc = Allocator()) : List(init.begin(), init.end(), alloc) { }

  // Assignment operators.
  List& operator=(List const& other) { assign(other.begin(), other.end()); return *this; }
  List& operator=(List&& other) noexcept { clear(); this->splice(this->end(), other); return *this; }

  // Destructor
  ~List() { clear(); }

  allocator_type get_allocator() const { return alloc_; }

  // Modifiers

  void clear() noexcept;

  template<typename... Args>
  iterator emplace(const_iterator pos, Args&&... args);

  template<typename... Args>
  reference emplace_back(Args&&... args);

  template<typename... Args>
  reference emplace_front(Args&&... args) { return *emplace(const_iterator{this->base_node_.next_}, std::forward<Args>(args)...); }

  // Insertion
  iterator insert(const_iterator pos, T const& value) { return emplace(pos, value); }
  iterator insert(const_iterator pos, T&& value) { return emplace(pos, std::move(value)); }
  iterator insert(const_iterator pos, size_type count, T const& value);
  template<typename InputIt>
  requires (std::convertible_to<std::iter_reference_t<InputIt>, T>)
  iterator insert(const_iterator pos, InputIt first, InputIt last);
  iterator insert(const_iterator pos, std::initializer_list<T> ilist) { return insert(pos, ilist.begin(), ilist.end()); }

  void assign(size_type count, T const& value) { clear(); insert(this->end(), count, value); }
  template<typename InputIt>
  requires (std::convertible_to<std::iter_reference_t<InputIt>, T>)
  void assign(InputIt first, InputIt last) { clear(); insert(this->end(), first, last); }
  void assign(std::initializer_list<T> ilist) { clear(); insert(ilist); }

  iterator erase(const_iterator position);
  iterator erase(const_iterator first, const_iterator last);

  void push_back(T const& value) { emplace_back(value); }
  void push_back(T&& value) { emplace_back(std::move(value)); }

  void push_front(T const& value) { emplace_front(value); }
  void push_front(T&& value) { emplace_front(std::move(value)); }

  void resize(size_type count);
  void resize(size_type count, T const& value);

  void swap(List& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value);

  template<typename UnaryPredicate>
  requires std::predicate<UnaryPredicate&, T const&>
  size_type remove_if(UnaryPredicate p);
  size_type remove(T const& value) { return remove_if([&value](T const& element) { return element == value; }); }

  template<typename BinaryPredicate>
  requires std::predicate<BinaryPredicate&, T const&, T const&>
  size_type unique(BinaryPredicate p);
  size_type unique() { return unique(std::equal_to<>()); }
};

template<typename T>
ListBase<T>::iterator ListBase<T>::value_type::prev()
{
  return typename ListBase<T>::iterator{untaint(this->prev_)};
}

template<typename T>
ListBase<T>::const_iterator ListBase<T>::value_type::prev() const
{
  return typename ListBase<T>::const_iterator{untaint(this->prev_)};
}

template<typename T>
ListBase<T>::iterator ListBase<T>::value_type::next()
{
  return typename ListBase<T>::iterator{this->next_};
}

template<typename T>
ListBase<T>::const_iterator ListBase<T>::value_type::next() const
{
  return typename ListBase<T>::const_iterator{this->next_};
}

template<typename T>
void ListBase<T>::splice(const_iterator pos, ListBase& other)
{
  if (AI_UNLIKELY(other.empty()))
    return;

  // Extract the elements of other.
  Node base_node = other.base_node_;

  // Update size_.
  size_ += other.size_;

  // Empty other.
  other.base_node_.next_ = &other.base_node_;
  other.base_node_.prev_ = ListBase<T>::taint(&other.base_node_);
  other.size_ = 0;

  // Insert all elements from other before pos.
  // From:
  //
  //   | base_node_      | other_begin     |                 | other_last      | base_node
  //                       |                                   |               |
  //   ____________________v_____________________........______v_______________|__________________
  //   |        next_ -->|        next_ -->|              -->|        next_ --->&other.base_node_
  //   | &other.base_node<--- prev_        |<--              |<-- prev_        |<-- prev_ (tainted)
  //
  //                       pos
  //   | pos_minus_one   | possibly_end    |
  //     |                 |
  //   __v_________________v________________
  //   |        next_ -->|                 |
  //   |                 |<-- prev_        |
  //
  // To:
  //                                                                             pos
  //   | pos_minus_one   | other_begin     |                 | other_last      | possibly_end    |
  //     |                 |                                   |                 |
  //   __v_________________v_____________________........______v_________________v________________
  //   |        next_ -->|        next_ -->|              -->|        next_ -->|                 |
  //   |                 |<-- prev_        |<--              |<-- prev_        |<-- prev_        |
  //
  Node* possibly_end = pos.ptr_;                        // pos can be end().
  uintptr_t extracted_taint;
  Node* pos_minus_one = extract_taint(possibly_end->prev_, extracted_taint);

  Node* other_begin = base_node.next_;                  // This is not &other.base_node_ because other is not empty.
  Node* other_last = untaint(base_node.prev_);          // Idem.

  link_node(pos_minus_one, other_begin);                // Connect pos_minus_one ⇄ other_begin.
  link_node(other_last, possibly_end, extracted_taint); // Connect    other_last ⇄ possibly_end.
}

template<typename T>
template<typename Compare>
void ListBase<T>::merge(ListBase& other, Compare comp)
{
  if (AI_UNLIKELY(this == &other))
    return;

  iterator tail = begin();
  iterator const end = this->end();
  iterator const other_end = other.end();

  while (!other.empty())
  {
    // Move tail forward till the first element larger than other.front().
    iterator other_front = other.begin();
    while (tail != end && !comp(*other_front, *tail))                                   // while (*tail <= *other_front)
      ++tail;
    // Now *tail > *other_front.

    // Move splice_end forward till the first element larger or equal than *tail.
    iterator splice_end = other_front;
    size_type range_size = 1;
    while (++splice_end != other_end && (tail == end || comp(*splice_end, *tail)))      // while (*splice_end < *tail)
      ++range_size;
    // Now *splice_end >= *tail.

    // Insert all elements from other that are less than *tail before tail.
    splice(tail, other, other_front, splice_end, range_size);
  }
}

template<typename T>
void ListBase<T>::reverse() noexcept
{
  for (Node* current = base_node_.next_; current != &base_node_; current = current->prev_)
    std::swap(current->next_, current->prev_);

  Node* begin = base_node_.next_;
  base_node_.next_ = untaint(base_node_.prev_);
  base_node_.prev_ = taint(begin);
}

#if DEBUG_MERGE_SORT
//static
template<typename T>
void ListBase<T>::dump_single_linked_list_on(std::ostream& os, typename ListBase<T>::WrapIterator wrap)
{
  iterator it = wrap.it_;
  os << '{';
  char const* separator = "";
  for (Node* node = it.ptr_; node; node = (--it).ptr_)
  {
    os << separator << *it;
    separator = ", ";
  }
  os << '}';
}

#define MergeDout(cntrl, data) \
    LibcwDout(LIBCWD_DEBUGCHANNELS, ::libcwd::libcw_do, cntrl, data)

#else // DEBUG_MERGE_SORT
#define MergeDout(cntrl, data) do { } while(0)
#endif // DEBUG_MERGE_SORT

template<typename T>
template<typename Compare>
requires std::predicate<Compare&, T const&, T const&>
void ListBase<T>::sort(Compare comp)
{
  // A merge sort implementation by Carlo Wood.

  // A single merge step exists of having a situation like the following:
  //
  //     P
  //     ⇓
  //     X → l0 → l1 → Y            }  next_ pointers
  //         ↓    ↓                 ⎫
  //         a    k                 ⎪
  //         ↓    ↓                 ⎪
  //         ⋮    ⋮                 ⎬  prev_ pointers
  //         ↓    ↓                 ⎪
  //         j    z                 ⎪
  //         ↓    ↓                 ⎭
  //      null    null
  //
  // where P points to an element (X) whose next_ pointer points to l0, the first
  // element of an ascending, sorted, singly-linked list, that uses the prev_ pointer
  // to point to the next (smaller) element (a) and terminates with a node (j) that has
  // prev_ == nullptr.
  //
  // The next_ pointer of l0 must point to another node (l1) which in turn is the
  // first element of an ascending, sorted, singly-linked list, that uses the prev_
  // pointer to point to the next (smaller) element (k) and also terminates with a
  // node (z) that has prev_ == nullptr.
  //
  // The step exists of merging l0 and l1 into a single, ascending, sorted,
  // singly-linked list (lₙ - which is the larger of l0 and l1) and update P,
  // the next_ pointer of X and lₙ.
  //
  // Hence, after the merge step we should end up with:
  //
  //         P
  //         ⇓
  //     X → lₙ → Y
  //         ↓
  //         ⋮
  //         ↓
  //        null
  //
  // where lₙ = *l1 < *l0 ? l0 : l1; so that we'll get l1 if l0 and l1 are equal (to preserve the same order).
  //
  // Hence, P must initially point to the node_base_.
  // The algoritm then repeats
  //
  //   X->next_ = P = merge(l0, l1);        // Where `merge` merges l0 and l1 and return lₙ.
  //   P->next_ = l1->next_;                // Make the top element (lₙ above) point to Y.
  //
  // until P->next_ points to base_node_ (Y is base_node_), or P->next_->next_ points to
  // base_node_, at which point we have to reset P to point to the base_node_ again.
  //
  // This terminating condition at the *beginning* of a new step is therefore:
  //
  //     P
  //     ⇓
  //     X → B
  //
  // or
  //
  //     P
  //     ⇓
  //     X → l0 → B
  //         ↓
  //         ⋮
  //         ↓
  //      null
  //
  //
  // In other words, either l0 or l1 are equal to B.
  //
  // Note that if the list is fully sorted then X is base_node_ too, hence
  // at the start of the next step both X and l1 are the base_node_.

  // B points to the base_node_.
  iterator B{*this};
  ASSERT(B.is_end());

  // We must begin by bringing the nodes of the list into the above described state.
  // It would be sufficient to set all prev_ pointers to null, but since that means
  // we need to run over all elements, we might as well combine that with the first
  // pass.
  //
  // That is, for every element pair, we want to go from:
  //
  //   | previous node   | first           | second          | next Node       |
  //   _________________________________________________________________________
  //   |        next_ -->|        next_ -->|        next_ -->|                 |
  //   |<-- prev_        |<-- prev_        |<-- prev_        |                 |
  //
  // if second < first, to
  //
  //   | previous node   | first           |                 | next Node       |
  //   _____________________________________                 ___________________
  //   |        next_ -->|        next_ ==========(1)=======>|                 |
  //   |<-- prev_        | prev_           |                 |                 |
  //                        ‖(2)
  //                        v
  //                     | second          |
  //                     __________________
  //                     |                 |
  //                     | prev_           |
  //                        ‖(3)
  //                        v
  //                     nullptr
  //
  // otherwise (first <= second) to
  //
  //   | previous node   |                 | second          | next Node       |
  //   __________________|                 _____________________________________
  //   |        next_ ==========(4)=======>|        next_ -->|                 |
  //   |<-- prev_        |                 | prev_           |                 |
  //                                          |
  //                                          v
  //                                       | first           |
  //                                       ___________________
  //                                       |                 |
  //                                       | prev_           |
  //                                          ‖(5)
  //                                          v
  //                                       nullptr
  //
  // If there are an odd number of nodes, then the last one must have its prev_ set to nullptr.
  //
  iterator previous_node = B;
  iterator first{previous_node.ptr_->next_};
  iterator second{first.ptr_->next_};

  // If the list contains less than two elements, do nothing.
  if (AI_UNLIKELY(second == B))
    return;

  do
  {
    iterator next{second.ptr_->next_};

    if (comp(*second, *first))
    {
      first->next_ = next.ptr_;                 // (1)
      first->prev_ = second.ptr_;               // (2)
      second->prev_ = nullptr;                  // (3)
      previous_node = first;
    }
    else
    {
      previous_node->next_ = second.ptr_;       // (4)
      first->prev_ = nullptr;                   // (5)
      previous_node = second;
    }
    first.ptr_ = previous_node.ptr_->next_;
    second.ptr_ = first.ptr_->next_;
  }
  while (first != B && second != B);
  if (first != B)
    first.ptr_->prev_ = nullptr;

  // We had at least two elements, so we have at least one singly-linked list now.
  // If there is only one such list, then the list has two elements and we have the following situation:
  //
  //     B → l0 → B
  //         ↓
  //         S
  //         ↓
  //        null
  //
  int remaining_lists = 1;
  iterator l0 = std::next(B);
  iterator l1 = std::next(l0);
  for (Node* node = l1.ptr_; node != B.ptr_; node = node->next_)
    if (++remaining_lists == 3)        // Three or more lists is treated the same.
      break;

  // If the list only had two elements handle that now.
  if (remaining_lists == 1)
  {
    // We need to convert the above into:
    //   B ⇄ S ⇄ l0 ⇄ B
    Node* S = l0->prev_;
    S->prev_ = B.ptr_;
    B.ptr_->prev_ = taint(l0.ptr_);
    B->next_ = S;
    S->next_ = l0.ptr_;
    return;
  }

  std::array<iterator, 2> iters = {{l0, l1}};

  bool done = remaining_lists == 2;
  while (!done)
  {
    iterator P = B;                     // Reset P to the beginning.
    for (;;)
    {
      // l0, l1 and iters were already initialized: before entering the while loop and/or at the bottom of the while or for loop respectively.
      iterator Y = std::next(l1);

      //---------------------------------------------------------------
      // Merge the singly-linked lists l0 and l1, which are already sorted in ascending order.
      //
      // Only the prev_ pointer is used and the last element of both lists must have prev_ == nullptr.
      // Afterwards P is set to point to the largest (first) element (which will be either l0 or l1).
      //

      int initial_largest;

      // Test terminating condition.
      //
      // There are exactly two lists left:
      //
      //     P
      //     ⇓
      //     B → l0 → l1 → B
      //         ↓    ↓
      //         ⋮    ⋮
      //         ↓    ↓
      //      null    null
      //
      // In otherwords, Y == B, but so does P.
      //
      // There are more than two lists left, but not two additional lists that can be merged.
      //
      //         P
      //         ⇓
      //     W → X → l0 → B     if W = B, then there are two lists left, but in that case
      //         ↓   ↓          we can still simply reset P to B, in which case we'll end
      //         ⋮   ⋮          up in the above case and terminate then.
      //         ↓   ↓
      //      null   null
      //
      // In other words, l1 == B.
      //
      // Restart at the beginning if there do not exist two additional lists that can be merged.
      if (AI_UNLIKELY(l1 == B || Y == B))
      {
        if (P == B)
          done = true;
        break;
      }

      //=======================================================================
      // Now that B, P, l0, l1 and Y are initialized, the merge step starts here.
      //
      MergeDout(dc::notice, "Merging " <<
          utils::print_using(WrapIterator{l0}, dump_single_linked_list_on) << " and " <<
          utils::print_using(WrapIterator{l1}, dump_single_linked_list_on) << "):");

      initial_largest = comp(*l1, *l0) ? 0 : 1; // Index into iters that we will put the largest iterator into.
      MergeDout(dc::notice, "initial_largest = " << initial_largest <<
          "(" << *iters[initial_largest] << "); we will return l" << initial_largest << ".");

      int largest = initial_largest;
      iterator const largest_iter = iters[largest];

      iterator last;
      for (;;)
      {
        MergeDout(dc::notice, "Advancing l" << largest << " (skipping " << *iters[largest] << ").");
        // Advance the smaller iterator.
        last = iters[largest]--;
        Node* last_node = last.ptr_;
        if (last_node->prev_ == nullptr)        // Was last the last element in the list?
        {
          MergeDout(dc::notice, "This was the last element of l" << largest << ".");
          break;
        }
        MergeDout(dc::notice, "Comparing: " << *iters[0] << " < " << *iters[1]);
        int next_largest = comp(*iters[1], *iters[0]) ? 0 : 1;
        if (next_largest != largest)
        {
          MergeDout(dc::notice, "Switching to l" << next_largest << " because " << *iters[next_largest] << " is the largest.");
          MergeDout(dc::notice, "Adding a link from " << *last << " to " << *iters[next_largest] << ".");
          largest = next_largest;
          last_node->prev_ = iters[largest].ptr_;
        }
        else
          MergeDout(dc::notice, "Staying on l" << largest << " because " << *iters[largest] << " is the largest.");
      }

      // Append the list with remaining elements after last.
      Node* last_node = last.ptr_;
      MergeDout(dc::notice, "Adding a link from " << *last << " to " << *iters[1 - largest] << ".");
      last_node->prev_ = iters[1 - largest].ptr_;

      MergeDout(dc::notice, "Result: " << utils::print_using(WrapIterator{largest_iter}, dump_single_linked_list_on));

      // End of merging of l0 and l1.
      //---------------------------------------------------------------

      Node* X = P.ptr_;
      P = largest_iter;         // Set P to point to the first element in the mergest list (lₙ).
      Node* ln = P.ptr_;
      X->next_ = ln;            // Update X->next_ to point to lₙ.
      ln->next_ = Y.ptr_;       // Update ln->next_ to point to Y.
      l0 = std::next(P);
      l1 = std::next(l0);       // Note: if l0 == B, then this returns begin.
      iters = {{l0, l1}};

      // Merge step ends here.
      //=======================================================================
    }
    l0 = std::next(B);
    l1 = std::next(l0);
    iters = {{l0, l1}};
  }

  // Now there are two remaining lists, we'll merge those while fixing the prev_ pointers too.
  MergeDout(dc::notice, "Merging the two final lists.");

  int initial_largest = comp(*l1, *l0) ? 0 : 1;     // Index into iters that we will put the largest iterator into.
  int largest = initial_largest;
  iterator const largest_iter = iters[largest];
  Node* next_node = B.ptr_;
  Node* last_node;
  for (;;)
  {
    last_node = (iters[largest]--).ptr_;
    link_node(last_node, next_node);
    next_node = last_node;
    if (last_node->prev_ == nullptr)    // Break out of loop once `largest` becomes empty.
      break;
    largest = comp(*iters[1], *iters[0]) ? 0 : 1;
  }
  // Attach remaining nodes from the list that wasn't empty yet.
  last_node = iters[1 - largest].ptr_;
  do
  {
    link_node(last_node, next_node);
    next_node = last_node;
    last_node = last_node->prev_;
  }
  while (last_node);
  B.ptr_->next_ = next_node;
  B.ptr_->prev_ = taint(largest_iter.ptr_);

  MergeDout(dc::notice, "Result: " << utils::print_using(WrapIterator{largest_iter}, dump_single_linked_list_on));
}

// Non-member function.
template<typename T, typename Allocator>
void swap(List<T, Allocator>& lhs, List<T, Allocator>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
  lhs.swap(rhs);
}

template<typename T2, typename Allocator2>
bool operator==(List<T2, Allocator2> const& lhs, List<T2, Allocator2> const& rhs)
{
  if (lhs.size() != rhs.size())
    return false;

  auto it1 = lhs.begin();
  auto it2 = rhs.begin();

  while (it1 != lhs.end())
  {
    if (!(*it1 == *it2))
      return false;
    ++it1;
    ++it2;
  }

  return true;
}

template<typename T, typename Allocator>
template<typename... Args>
List<T, Allocator>::node_type* List<T, Allocator>::create_node(Args&&... args)
{
  value_type* new_node = std::allocator_traits<Allocator>::allocate(alloc_, 1);
  std::allocator_traits<Allocator>::construct(alloc_, new_node, std::forward<Args>(args)...);
  return ListBase<T>::node_cast(new_node);
}

template<typename T, typename Allocator>
template<typename... Args>
List<T, Allocator>::iterator List<T, Allocator>::emplace(const_iterator pos, Args&&... args)
{
  node_type* new_node = create_node(std::forward<Args>(args)...);

  if (AI_UNLIKELY(this->is_end(pos)))
    this->push_back_node(new_node);
  else
    this->insert_node(pos, new_node);

  return List<T, Allocator>::iterator{new_node};
}

template<typename T, typename Allocator>
template<typename... Args>
List<T, Allocator>::reference List<T, Allocator>::emplace_back(Args&&... args)
{
  node_type* new_node = create_node(std::forward<Args>(args)...);
  this->push_back_node(new_node);
  return *ListBase<T>::value_type_cast(new_node);
}

template<typename T, typename Allocator>
void List<T, Allocator>::clear() noexcept
{
  // Run over all elements once.
  for (size_type n = this->size_; n != 0; --n)
  {
    value_type* node = ListBase<T>::value_type_cast(this->extract_front());
    std::allocator_traits<Allocator>::destroy(alloc_, node);
    std::allocator_traits<Allocator>::deallocate(alloc_, node, 1);
  }
  this->base_node_.prev_ = ListBase<T>::taint(&this->base_node_);
  this->size_ = 0;
}

template<typename T, typename Allocator>
List<T, Allocator>::List(List const& other) : alloc_(std::allocator_traits<Allocator>::select_on_container_copy_construction(other.alloc_))
{
  try
  {
    for (auto const& item : other)
      push_back(item);
  }
  catch (...)
  {
    clear();
    throw;
  }
}

template<typename T, typename Allocator>
List<T, Allocator>::List(List&& other) : ListBase<T>(std::move(other)), alloc_(std::move(other.alloc_))
{
  if (this != &other)
  {
    // Update the pointers of the first and last nodes.
    if (AI_UNLIKELY(this->empty()))
    {
      this->base_node_.next_ = &this->base_node_;
      this->base_node_.prev_ = ListBase<T>::taint(&this->base_node_);
    }
    else
    {
      this->base_node_.next_->prev_ = &this->base_node_;
      ListBase<T>::untaint(this->base_node_.prev_)->next_ = &this->base_node_;
    }

    // Reset the other list.
    other.base_node_.next_ = &other.base_node_;
    other.base_node_.prev_ = ListBase<T>::taint(&other.base_node_);
    other.size_ = 0;
  }
}

template<typename T, typename Allocator>
template<typename InputIt>
requires (std::convertible_to<std::iter_reference_t<InputIt>, T>)
List<T, Allocator>::List(InputIt first, InputIt last, Allocator const& alloc) : alloc_(alloc)
{
  node_type* tail = &this->base_node_;
  size_type count = 0;
  for (InputIt current = first; current != last; ++current)
  {
    node_type* new_node = create_node(static_cast<T const&>(*current));
    this->link_node(tail, new_node);
    tail = new_node;
    ++count;
  }
  this->link_node(tail, &this->base_node_, 1);
  this->size_ += count;
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_type count, T const& value, Allocator const& alloc) : alloc_(alloc)
{
  node_type* tail = &this->base_node_;
  for (int i = 0; i < count; ++i)
  {
    node_type* new_node = create_node(value);
    this->link_node(tail, new_node);
    tail = new_node;
  }
  this->link_node(tail, &this->base_node_, 1);
  this->size_ += count;
}

template<typename T, typename Allocator>
List<T, Allocator>::List(size_type count, Allocator const& alloc) : alloc_(alloc)
{
  node_type* tail = &this->base_node_;
  for (int i = 0; i < count; ++i)
  {
    node_type* new_node = create_node();
    this->link_node(tail, new_node);
    tail = new_node;
  }
  this->link_node(tail, &this->base_node_, 1);
  this->size_ += count;
}

template<typename T, typename Allocator>
void List<T, Allocator>::swap(List& other) noexcept(std::allocator_traits<Allocator>::is_always_equal::value)
{
  std::swap(static_cast<ListBase<T>&>(*this), static_cast<ListBase<T>&>(other));
  std::swap(alloc_, other.alloc_);
}

template<typename T, typename Allocator>
List<T, Allocator>::iterator List<T, Allocator>::insert(const_iterator pos, size_type count, T const& value)
{
  if (AI_UNLIKELY(count == 0))
    return std::bit_cast<List<T, Allocator>::iterator>(pos);

  // Insert first element.
  iterator first = emplace(pos, value);

  // Insert remaining elements, if any.
  for (size_type i = 1; i < count; ++i)
    emplace(pos, value);

  return first;
}

template<typename T, typename Allocator>
template<typename InputIt>
requires (std::convertible_to<std::iter_reference_t<InputIt>, T>)
List<T, Allocator>::iterator List<T, Allocator>::insert(const_iterator pos, InputIt first, InputIt current)
{
  // current points one past the end.
  if (AI_UNLIKELY(first == current))
    return std::bit_cast<List<T, Allocator>::iterator>(pos);

  // Insert all elements, starting at the end, except the first one.
  while (--current != first)
    pos = emplace(pos, static_cast<const_reference>(*current));

  // Insert first element.
  return emplace(pos, static_cast<const_reference>(*current));
}

template<typename T, typename Allocator>
List<T, Allocator>::iterator List<T, Allocator>::erase(const_iterator position)
{
  // Point past the element that must be erased.
  bool const is_first_element = position.is_begin();
  ++position;
  --(this->size_);

  // Get and remove the node that must be erased from the list.
  node_type* node;
  if (AI_UNLIKELY(position.is_end()))
    node = this->extract_back();
  else if (AI_UNLIKELY(is_first_element))
  {
    node = this->extract_front();
    if (AI_UNLIKELY(this->size_ == 0))
      this->base_node_.prev_ = ListBase<T>::taint(&this->base_node_);
  }
  else
    node = this->extract_node_before(position);

  value_type* value = ListBase<T>::value_type_cast(node);
  std::allocator_traits<Allocator>::destroy(alloc_, value);
  std::allocator_traits<Allocator>::deallocate(alloc_, value, 1);

  return std::bit_cast<List<T, Allocator>::iterator>(position);
}

template<typename T, typename Allocator>
List<T, Allocator>::iterator List<T, Allocator>::erase(const_iterator first, const_iterator last)
{
  if (AI_LIKELY(first != last))
  {
    // Remove the specified range from the list. Afterwards the list [first, last_extracted] are the removed elements.
    this->extract_range(first, last);
    const_iterator current = first;
    size_type count = 0;
    do
    {
      value_type* value = const_cast<value_type*>(current.operator->());
      ++current;
      ++count;
      std::allocator_traits<Allocator>::destroy(alloc_, value);
      std::allocator_traits<Allocator>::deallocate(alloc_, value, 1);
    }
    while (current != last);
    this->size_ -= count;
  }
  return std::bit_cast<List<T, Allocator>::iterator>(last);
}

template<typename T, typename Allocator>
void List<T, Allocator>::resize(size_type count)
{
  if (count >= this->size_)
  {
    // Append count - size_ default constructed elements.
    for (size_type i = this->size_; i < count; ++i)
      this->push_back_node(create_node());
    return;
  }

  erase(this->element_after(count), this->end());
}

template<typename T, typename Allocator>
void List<T, Allocator>::resize(size_type count, T const& value)
{
  if (count >= this->size_)
  {
    // Append count - size_ times an element constructed from value.
    for (size_type i = this->size_; i < count; ++i)
      this->push_back_node(create_node(value));
    return;
  }

  erase(this->element_after(count), this->end());
}

template<typename T, typename Allocator>
template<typename UnaryPredicate>
requires std::predicate<UnaryPredicate&, T const&>
List<T, Allocator>::size_type List<T, Allocator>::remove_if(UnaryPredicate p)
{
  size_type count = 0;
  auto it = this->begin();

  while (it != this->end())
  {
    if (p(*it))
    {
      it = erase(it);
      ++count;
    }
    else
      ++it;
  }

  return count;
}

template<typename T, typename Allocator>
template<typename BinaryPredicate>
requires std::predicate<BinaryPredicate&, T const&, T const&>
List<T, Allocator>::size_type List<T, Allocator>::unique(BinaryPredicate p)
{
  size_type count = 0;

  auto prev = this->begin();
  auto it = std::next(prev);    // This relies on the fact that incrementing end gives again end if the list is empty, for this List.

  while (it != this->end())
  {
    if (p(*prev, *it))
    {
      it = erase(it);
      ++count;
    }
    else
      prev = it++;
  }

  return count;
}

template<typename T2, typename Allocator2>
auto operator<=>(List<T2, Allocator2> const& lhs, List<T2, Allocator2> const& rhs)
{
  auto it1 = lhs.begin();
  auto it2 = rhs.begin();
  auto end1 = lhs.end();
  auto end2 = rhs.end();

  while (it1 != end1 && it2 != end2)
  {
    if (auto cmp = *it1 <=> *it2; cmp != 0)
      return cmp;
    ++it1;
    ++it2;
  }

  return lhs.size() <=> rhs.size();
}

} // namespace utils
