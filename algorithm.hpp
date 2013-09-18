#pragma once
#ifndef ALGORITHM_HPP
# define ALGORUTHM_HPP

#include <type_traits>

#include <utility>

namespace detail
{

template <bool B>
using bool_ = ::std::integral_constant<bool, B>;

template <class A, class ...B>
struct all_of : bool_<A{} && all_of<B...>{}> { };

template <class A>
struct all_of<A> : bool_<A::value> { };

}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<!bool(sizeof...(A)), T>::type
max(T const a, T const b, A const ...args)
{
  return a > b ? a : b;
}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<bool(sizeof...(A)) &&
  ::detail::all_of<::std::is_same<A, T>...>{}, T>::type
max(T const a, T const b, A const ...args)
{
  return a > b ? max(a, args...) : min(b, args...);
}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<!bool(sizeof...(A)), T>::type
min(T const a, T const b, A const ...args)
{
  return a < b ? a : b;
}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<bool(sizeof...(A)) &&
  ::detail::all_of<::std::is_same<A, T>...>{}, T>::type
min(T const a, T const b, A const ...args)
{
  return a < b ? min(a, args...) : min(b, args...);
}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<!bool(sizeof...(A)),
  ::std::pair<T, T> >::type
min(T const a, T const b, A const ...args)
{
  return a < b ? a : b;
}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<bool(sizeof...(A)) &&
  ::detail::all_of<::std::is_same<A, T>...>{},
  ::std::pair<T, T> >::type
minmax(T const a, T const b, A const ...args)
{
  return ::std::pair<T, T>(
    a < b ? min(a, args...) : min(b, args...),
    a > b ? max(a, args...) : max(b, args...));
}


#endif // ALGORITHM_HPP
