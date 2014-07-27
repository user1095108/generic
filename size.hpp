#ifndef GENERIC_SIZE_HPP
# define GENERIC_SIZE_HPP
# pragma once

namespace generic
{

// size
template <typename T, ::std::size_t N>
inline constexpr decltype(N) size(T const (&)[N]) noexcept
{
  return N;
}

}

#endif // GENERIC_SIZE_HPP
