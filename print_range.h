#pragma once

#include <iterator>
#include <concepts>

namespace utils {

template<typename T>
concept ConceptReferenceable = !std::is_void_v<std::remove_cv_t<T>>;

template<class I>
concept ConceptLegacyIterator =
    std::copyable<I> &&
    requires(I i)
    {
      {   *i } -> ConceptReferenceable;
      {  ++i } -> std::same_as<I&>;
      { *i++ } -> ConceptReferenceable;
    };

template <class I>
concept ConceptLegacyInputIterator =
    ConceptLegacyIterator<I> &&
    std::equality_comparable<I> &&
    requires(I i)
    {
      typename std::incrementable_traits<I>::difference_type;
      typename std::indirectly_readable_traits<I>::value_type;
      typename std::common_reference_t<std::iter_reference_t<I>&&,
                                       typename std::indirectly_readable_traits<I>::value_type&>;
      *i++;
      typename std::common_reference_t<decltype(*i++)&&,
                                       typename std::indirectly_readable_traits<I>::value_type&>;
      requires std::signed_integral<typename std::incrementable_traits<I>::difference_type>;
    };

template <class It>
concept ConceptLegacyForwardIterator =
    ConceptLegacyInputIterator<It> && std::constructible_from<It> &&
    std::is_reference_v<std::iter_reference_t<It>> &&
    std::same_as<
        std::remove_cvref_t<std::iter_reference_t<It>>,
        typename std::indirectly_readable_traits<It>::value_type> &&
    requires(It it)
    {
      {  it++ } -> std::convertible_to<It const&>;
      { *it++ } -> std::same_as<std::iter_reference_t<It>>;
    };

template <ConceptLegacyForwardIterator ForwardIterator>
struct PrintRange1
{
  ForwardIterator first_;
  ForwardIterator last_;
};

template <ConceptLegacyForwardIterator ForwardIterator>
inline PrintRange1<ForwardIterator> print_range(ForwardIterator first, ForwardIterator last)
{
  return {first, last};
}

// To be used as follows:
//
//   Dout(dc::notice, "This is a range:" << utils::print_range(v.begin(), v.end()));
//
template <ConceptLegacyForwardIterator ForwardIterator>
std::ostream& operator<<(std::ostream& os, PrintRange1<ForwardIterator> range)
{
  os << '{';
  char const* separator = "";
  while (range.first_ != range.last_)
  {
    os << separator << *range.first_;
    separator = ", ";
    ++range.first_;
  }
  os << '}';
  return os;
}

template <ConceptLegacyForwardIterator ForwardIterator, typename PF>
requires std::invocable<PF&, std::ostream&, std::iter_reference_t<ForwardIterator>>
struct PrintRange2
{
  ForwardIterator first_;
  ForwardIterator last_;
  PF&& fp_; // Forward reference to invocable function.
};

template <ConceptLegacyForwardIterator ForwardIterator, typename PF>
requires std::invocable<PF&, std::ostream&, std::iter_reference_t<ForwardIterator>>
inline PrintRange2<ForwardIterator, PF&&> print_range(ForwardIterator first, ForwardIterator last, PF&& pf)
{
  return {first, last, std::forward<PF>(pf)};
}

// To be used as follows:
//
//   Dout(dc::notice, "This is a range:" << utils::print_range(v.begin(), v.end(), [](std::ostream& os, Foo const& foo){ os << foo; }));
//
template <ConceptLegacyForwardIterator ForwardIterator, typename PF>
std::ostream& operator<<(std::ostream& os, PrintRange2<ForwardIterator, PF> range)
{
  os << '{';
  char const* separator = "";
  while (range.first_ != range.last_)
  {
    os << separator;
    separator = ", ";
    std::invoke(range.fp_, os, *range.first_);
    ++range.first_;
  }
  os << '}';
  return os;
}

} // namespace utils
