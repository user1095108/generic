#ifndef GENERIC_DBG_HPP
# define GENERIC_DBG_HPP
# pragma once

#include <ostream>

#include <iostream>

namespace generic
{

#ifdef NDEBUG
static struct
{
  template <typename U>
  auto& operator<<(U&&) const noexcept
  {
    return *this;
  }

  auto& operator<<(::std::ostream&(* const)(::std::ostream&)) const noexcept
  {
    return *this;
  }
} const dbg;
#else
static constexpr auto& dbg(::std::cout);
#endif // NDEBUG

}

#endif // GENERIC_DBG_HPP
