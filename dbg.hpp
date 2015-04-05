#ifndef GENERIC_DBG_HPP
# define GENERIC_DBG_HPP
# pragma once

#include <iostream>

namespace generic
{

#ifndef NDEBUG
static decltype((::std::cout)) dbg(::std::cout);
#else
static struct
{
  template <typename U>
  decltype(auto) operator<<(U&&) const noexcept
  {
    return *this;
  }

  decltype(auto) operator<<(
    ::std::ostream& (*)(::std::ostream&)
  ) const noexcept
  {
    return *this;
  }
} dbg;

#endif // NDEBUG

}

#endif // GENERIC_DBG_HPP
