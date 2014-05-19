#ifndef ALGORITHM_HPP
# define ALGORUTHM_HPP
# pragma once

#include <cstddef>

#include <type_traits>

#include <utility>

#include "utility.hpp"

namespace generic
{

// min, max
template <typename T>
inline constexpr T max(T a, T b)
{
  return a > b ? a : b;
}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<bool(sizeof...(A)) &&
  all_of<::std::is_same<A, T>...>{}, T>::type
max(T a, T b, A ...args)
{
  return a > b ? max(a, args...) : max(b, args...);
}

template <typename T>
inline constexpr T min(T a, T b)
{
  return a < b ? a : b;
}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<bool(sizeof...(A)) &&
  all_of<::std::is_same<A, T>...>{}, T>::type
min(T a, T b, A ...args)
{
  return a < b ? min(a, args...) : min(b, args...);
}

template <typename ...A>
inline constexpr typename ::std::enable_if<bool(sizeof...(A)) &&
  all_of<::std::is_same<typename front<A...>::type, A>...>{},
  ::std::pair<typename front<A...>::type,
    typename front<A...>::type> >::type
minmax(A ...args)
{
  return {min(args...), max(args...)};
}

}

#endif // ALGORITHM_HPP
