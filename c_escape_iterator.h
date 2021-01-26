#pragma once

#include <boost/iterator/iterator_adaptor.hpp>
#include <cstring>

namespace utils {

template<class IteratorBase>
class c_escape_iterator : public boost::iterator_adaptor<
    c_escape_iterator<IteratorBase>,     // The derived class overriding iterator behavior.
    IteratorBase,               // The base class providing default behavior.
    boost::use_default,         // Iterator value type will be IteratorBase::value_type.
    std::forward_iterator_tag,  // Iterator category.
    boost::use_default          // Iterator reference type.
    >
{
 private:
  IteratorBase const m_end;
  char m_escape_buf[6];

 private:
  void prepare_escape_buf();

 public:
  // Construct an iterator that points to itBegin.
  // itEnd must be an iterator that will be reached by repeatedly incrementing itBegin.
  c_escape_iterator(IteratorBase itBegin, IteratorBase itEnd) : c_escape_iterator::iterator_adaptor_(itBegin), m_end(itEnd), m_escape_buf{}
  {
    if (this->base_reference() != m_end)
      prepare_escape_buf();
  }

  // A c_escape_iterator constructed with this constructor may never be incremented or dereferenced,
  // even if iterator that is the passed is capable to do that. You may only use this
  // to compare with.
  c_escape_iterator(IteratorBase itEnd) : c_escape_iterator::iterator_adaptor_(itEnd), m_end{} { }

  void increment()
  {
    if (*m_escape_buf)
    {
      if (m_escape_buf[++*m_escape_buf])
        return;
      *m_escape_buf = 0;
    }

    if (++this->base_reference() != m_end)
      prepare_escape_buf();
  }

  typename c_escape_iterator::reference dereference() const
  {
    if (*m_escape_buf)
      return m_escape_buf[*m_escape_buf];
    return *this->base_reference();
  }
};


template<class IteratorBase>
void c_escape_iterator<IteratorBase>::prepare_escape_buf()
{
  char c = *this->base_reference();

  if (c > 31 && c != 92 && c != 127)
      return;

  m_escape_buf[1] = '\\';
  *m_escape_buf = 1;
  if (c > 6 && c < 14)
  {
    static char const* c2s_tab = "abtnvfr";
    m_escape_buf[2] = c2s_tab[c - 7];
    m_escape_buf[3] = 0;
  }
  else if (c == 27)
  {
    m_escape_buf[2] = 'e';
    m_escape_buf[3] = 0;
  }
  else if (c == '\\')
  {
    m_escape_buf[2] = '\\';
    m_escape_buf[3] = 0;
  }
  else
  {
    m_escape_buf[2] = 'x';
    int xval = (unsigned char)c;
    int hd = xval / 16;
    for (int i = 3; i < 5; ++i)
    {
      m_escape_buf[i] = (hd < 10) ? '0' + hd : 'A' + hd - 10;
      hd = xval % 16;
    }
    m_escape_buf[5] = 0;
  }
}

} // namespace utils
