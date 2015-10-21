#ifndef ALGORITHM_HPP
# define ALGORUTHM_HPP
# pragma once

#include <cstddef>

#include <type_traits>

#include <utility>

#include "meta.hpp"

namespace generic
{

// min, max
template <typename T>
constexpr inline T const& max(T const& a, T const& b) noexcept
{
  return a > b ? a : b;
}

template <typename T, typename ...A>
constexpr inline typename ::std::enable_if<
  bool(sizeof...(A)) &&
  all_of<
    ::std::is_same<
      typename ::std::decay<T>::type,
      typename ::std::decay<A>::type
    >...
  >{},
  T
>::type
max(T const a, T const b, A&& ...args) noexcept
{
  return a > b ?
    max(a, ::std::forward<A>(args)...) :
    max(b, ::std::forward<A>(args)...);
}

template <typename T>
constexpr inline T const& min(T const& a, T const& b) noexcept
{
  return a < b ? a : b;
}

template <typename T, typename ...A>
constexpr inline typename ::std::enable_if<
  bool(sizeof...(A)) &&
  all_of<
    ::std::is_same<
      typename ::std::decay<T>::type,
      typename ::std::decay<A>::type
    >...
  >{},
  T
>::type
min(T const a, T const b, A&& ...args) noexcept
{
  return a < b ?
    min(a, ::std::forward<A>(args)...) :
    min(b, ::std::forward<A>(args)...);
}

template <typename ...A>
constexpr inline typename ::std::enable_if<
  bool(sizeof...(A)) &&
  all_of<
    ::std::is_same<
      typename ::std::decay<typename front<A...>::type>::type,
      typename ::std::decay<A>::type
    >...
  >{},
  ::std::pair<
    typename ::std::decay<typename front<A...>::type>::type,
    typename ::std::decay<typename front<A...>::type>::type
  >
>::type
minmax(A&& ...args) noexcept
{
  return {
    min(::std::forward<A>(args)...),
    max(::std::forward<A>(args)...)
  };
}

}

#endif // ALGORITHM_HPP
