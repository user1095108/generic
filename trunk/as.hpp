#ifndef GENERIC_AS_HPP
# define GENERIC_AS_HPP
# pragma once

namespace generic
{

// as_const
//////////////////////////////////////////////////////////////////////////////
template<typename T>
inline constexpr T const& as_const(T& t) noexcept
{
  return t;
}

}

#endif // GENERIC_AS_HPP
