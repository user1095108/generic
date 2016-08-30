#ifndef TRIVIAL_TUPLE_HPP
# define TRIVIAL_TUPLE_HPP
# pragma once

namespace generic
{

template <::std::size_t I, typename T, typename ...A>
struct tuple_element : tuple_element<I + 1, T, ...A>
{
  T value;
};

template <::std::size_t I, typename T, typename ...A>
struct tuple_element<I, T&, A...> : tuple_element<I + 1, T, ...A>
{
  T* value;
};

template <typename ...A>
struct tuple : tuple_element<0, A...>
{
  A value;
};

template <typename ...A, ::std::size_t ...Is>
auto make_tuple(A&&... a, ::std::index_sequence<Is...> const)
{
  static_cast<tuple_element
}

template <::std::size_t I, typename ...A>
decltype(auto) get(tuple<A...> const& t) noexcept
{
  return static_cast<tuple_subtype_t<I, A...> const&>(t).value;
}

template <::std::size_t I, typename ...A>
decltype(auto) get(tuple<A...>& t) noexcept
{
  return static_cast<tuple_subtype_t<I, A...> const&>(t).value;
}

}

#endif // TRIVIAL_TUPLE_HPP
