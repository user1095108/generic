#ifndef GNR_ALGORITHM_HPP
# define GNR_ALGORITHM_HPP
# pragma once

#include <cstddef>

#include <type_traits>

#include <utility>

#include "meta.hpp"

namespace gnr
{

// min, max
template <typename T>
constexpr inline T const& max(T const& a, T const& b) noexcept
{
  return b < a ? a : b;
}

template <typename T>
constexpr inline T const& min(T const& a, T const& b) noexcept
{
  return a < b ? a : b;
}

template <typename T, typename ...A>
constexpr inline std::enable_if_t<
  bool(sizeof...(A)) &&
  all_of<
    std::is_same<
      typename std::decay_t<T>,
      typename std::decay_t<A>
    >...
  >{},
  T
>
max(T const& a, T const& b, A&& ...args) noexcept
{
  return b < a ?
    max(a, std::forward<A>(args)...) :
    max(b, std::forward<A>(args)...);
}

template <typename T, typename ...A>
constexpr inline std::enable_if_t<
  bool(sizeof...(A)) &&
  all_of<
    std::is_same<
      typename std::decay_t<T>,
      typename std::decay_t<A>
    >...
  >{},
  T
>
min(T const& a, T const& b, A&& ...args) noexcept
{
  return a < b ?
    min(a, std::forward<A>(args)...) :
    min(b, std::forward<A>(args)...);
}

template <typename ...A>
constexpr inline std::enable_if_t<
  bool(sizeof...(A)) &&
  all_of<
    std::is_same<
      typename std::decay_t<typename front<A...>::type>,
      typename std::decay_t<A>
    >...
  >{},
  std::pair<
    typename std::decay_t<typename front<A...>::type>,
    typename std::decay_t<typename front<A...>::type>
  >
>
minmax(A&& ...args) noexcept
{
  return {
    min(std::forward<A>(args)...),
    max(std::forward<A>(args)...)
  };
}

template<typename T, typename U, typename V>
constexpr inline std::enable_if_t<
  std::is_same<T, U>{} &&
  std::is_same<U, V>{},
  T const&
>
clamp(T const& v, U const& lo, V const& hi) noexcept
{
  return v < lo ?
    lo :
    hi < v ?
      hi :
      v;
}

}

#endif // GNR_ALGORITHM_HPP
