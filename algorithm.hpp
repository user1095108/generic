#pragma once
#ifndef ALGORITHM_HPP
# define ALGORUTHM_HPP

#include <cstddef>

#include <type_traits>

#include <utility>

namespace detail
{

template <::std::size_t I, typename A, typename ...B>
struct type_at : type_at<I - 1, B...>
{
};

template <typename A, typename ...B>
struct type_at<0, A, B...>
{
  using type = A;
};

template <typename A, typename ...B>
struct front
{
  using type = A;
};

template <typename A, typename ...B>
struct back : back<B...>
{
};

template <typename A>
struct back<A>
{
  using type = A;
};

template <bool B>
using bool_ = ::std::integral_constant<bool, B>;

template <class A, class ...B>
struct all_of : bool_<A::value && all_of<B...>::value> { };

template <class A>
struct all_of<A> : bool_<A::value> { };

}

template <typename T>
inline constexpr T max(T a, T b)
{
  return a > b ? a : b;
}

template <typename T, typename ...A>
inline constexpr typename ::std::enable_if<bool(sizeof...(A)) &&
  ::detail::all_of<::std::is_same<A, T>...>{},
  T>::type
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
  ::detail::all_of<::std::is_same<A, T>...>{},
  T>::type
min(T a, T b, A ...args)
{
  return a < b ? min(a, args...) : min(b, args...);
}

template <typename ...A>
inline constexpr typename ::std::enable_if<bool(sizeof...(A)),
  ::std::pair<typename ::detail::front<A...>::type,
    typename ::detail::front<A...>::type> >::type
minmax(A ...args)
{
  return {min(args...), max(args...)};
}

#endif // ALGORITHM_HPP
