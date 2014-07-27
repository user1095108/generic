#ifndef GENERIC_META_HPP
# define GENERIC_META_HPP
# pragma once

#include <cstddef>

#include <type_traits>

namespace generic
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

template <class A, class ...B>
struct all_of : ::std::integral_constant<bool, A{} && all_of<B...>{}>
{
};

template <class A>
struct all_of<A> : ::std::integral_constant<bool, A{}>
{
};

}

#endif // GENERIC_META_HPP
