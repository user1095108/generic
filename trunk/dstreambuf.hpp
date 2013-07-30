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
    setp(buffer_, buffer_ + bufsize);
  }

private:
  int overflow(int const c)
  {
    sync();

    if (traits_type::eof() != c)
    {
      *pptr() = traits_type::to_char_type(c);

      pbump(1);
    }
    // else do nothing

    return traits_type::to_int_type(c);
  }

  int sync()
  {
    if (pbase() != pptr())
    {
      delegate_(pbase(), pptr() - pbase());

      setp(buffer_, buffer_ + bufsize);
    }
    // else do nothing

    return 0;
  }

private:
  delegate_type delegate_;

  char buffer_[bufsize];
};

#endif // DSTREAMBUF_HPP
