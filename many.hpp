#ifndef GNR_MANY_HPP
# define GNR_MANY_HPP
# pragma once

#include <cstddef>

#include <type_traits>

#include <utility>

namespace gnr
{

namespace detail
{

namespace many
{

enum TupleValue { Scalar, Reference, Class };

template <std::size_t I,
  class T,
  TupleValue = std::is_class<T>{} ?
    TupleValue::Class :
    std::is_reference<T>{} ?
      TupleValue::Reference :
      TupleValue::Scalar
>
struct tuple_base
{
  T value;

  tuple_base() = default;

  template <typename A>
  constexpr tuple_base(A&& value) : value{std::forward<A>(value)}
  {
  }

  tuple_base(tuple_base const&) = default;
  tuple_base(tuple_base&&) = default;
  tuple_base& operator=(tuple_base const&) = default;
  tuple_base& operator=(tuple_base&&) = default;
};

template <std::size_t I, class T>
struct tuple_base<I, T, TupleValue::Reference>
{
  T value;

  tuple_base() = delete;

  template <typename A>
  constexpr tuple_base(A&& value) : value{std::forward<A>(value)}
  {
  }

  tuple_base(tuple_base const&) = default;
  tuple_base(tuple_base&&) = default;
  tuple_base& operator=(tuple_base const&) = default;
  tuple_base& operator=(tuple_base&&) = default;
};

template <std::size_t I, class T>
struct tuple_base<I, T, TupleValue::Class> : T
{
  tuple_base() = default;

  template <typename A>
  constexpr tuple_base(A&& value) : T{std::forward<A>(value)}
  {
  }

  tuple_base(tuple_base const&) = default;
  tuple_base(tuple_base&&) = default;
  tuple_base& operator=(tuple_base const&) = default;
  tuple_base& operator=(tuple_base&&) = default;
};

template <std::size_t I, class T, enum detail::many::TupleValue E>
auto& get(tuple_base<I, T, E> const& obj) noexcept
{
  return obj.value;
}

template <std::size_t I, class T>
auto get(tuple_base<I, T, TupleValue::Class>& obj) noexcept -> T&
{
  return obj;
}

template <std::size_t I, class T>
auto get(tuple_base<I, T, TupleValue::Class> const& obj) noexcept -> T const&
{
  return obj;
}

template <typename ...>
struct many_impl;

template <std::size_t... Is, typename ...Types>
struct many_impl<std::index_sequence<Is...>, Types...> :
  tuple_base<Is, Types>...
{
  many_impl() = default;

  ~many_impl() = default;
  many_impl(many_impl const&) = default;
  many_impl(many_impl&&) = default;
  many_impl& operator=(many_impl const&) = default;
  many_impl& operator=(many_impl&&) = default;

#if !__cpp_aggregate_bases
  template<typename ...U>
  constexpr many_impl(U&& ...u) :
    tuple_base<Is, Types>{std::forward<U>(u)}...
  {
  }
#endif
};

}

}

template <typename ...Types>
struct many : detail::many::many_impl<
  std::make_index_sequence<sizeof...(Types)>,
  Types...
>
{
  many() = default;

  ~many() = default;
  many(many&&) = default;
  many(many const&) = default;
  many& operator=(many&&) = default;
  many& operator=(many const&) = default;

#if !__cpp_aggregate_bases
  template <class... U>
  constexpr many(U&& ...u) : many::many_impl{std::forward<U>(u)...}
  {
  }
#endif
};

}

namespace std
{

template <typename ...Types>
class tuple_size<gnr::many<Types...>> :
  public std::integral_constant<size_t, sizeof...(Types)>
{
};

template <size_t I, class ...Types>
class tuple_element<I, gnr::many<Types...>>
{
public:
  using type = tuple_element_t<I, tuple<Types...>>;
};

template <size_t I, typename ...Types> 
auto& get(gnr::many<Types...>& m) noexcept
{
  return gnr::detail::many::get<I>(m);
}

template <size_t I, typename ...Types> 
auto& get(gnr::many<Types...> const& m) noexcept
{
  return gnr::detail::many::get<I>(m);
}

template<size_t I, typename ...Types>
auto& get(gnr::many<Types...>&& m) noexcept
{
  // m is now a lvalue
  return gnr::detail::many::get<I>(m);
}

template<size_t I, typename ...Types>
auto& get(gnr::many<Types...> const&& m) noexcept
{
  // m is now a lvalue
  return gnr::detail::many::get<I>(m);
}

}

#endif // GNR_MANY_HPP
