#pragma once
#ifndef DSTREAMBUF_HPP
# define DSTREAMBUF_HPP

#include <streambuf>

#include "delegate.hpp"

template <std::size_t bufsize = 128>
struct dstreambuf : std::streambuf
{
  typedef delegate<void (char const*, std::size_t)> delegate_type;

  dstreambuf(delegate_type d)
    : delegate_(d)
  {
    super::setp(buffer_, buffer_ + bufsize);
  }

private:
  int overflow(int const c)
  {
    dstreambuf::sync();

    if (super::traits_type::eof() != c)
    {
      *super::pptr() = super::traits_type::to_char_type(c);

      super::pbump(1);
    }
    // else do nothing

    return traits_type::to_int_type(c);
  }

  int sync()
  {
    if (super::pbase() != super::pptr())
    {
      delegate_(super::pbase(), super::pptr() - super::pbase());

      super::setp(buffer_, buffer_ + bufsize);
    }
    // else do nothing

    return 0;
  }

  std::streamsize xsputn(char const* const s, std::streamsize const n)
  {
    if (n > 0)
    {
      delegate_(s, n);
    }
    // else do nothing

    return n;
  }

private:
  typedef std::streambuf super;

  delegate_type delegate_;

  char buffer_[bufsize];
};

#endif // DSTREAMBUF_HPP
