#include <cstddef>

#include <tuple>

#include <type_traits>

#include <utility>

namespace generic
{

namespace detail
{

namespace many
{

template <::std::size_t I, typename T>
struct many_holder
{
  ::std::conditional_t<::std::is_reference<T>{},
    ::std::add_pointer_t<::std::remove_reference_t<T>>,
    T
  > value;

  template <typename U = T>
  ::std::enable_if_t<!::std::is_reference<U>{}, U&>
  get() noexcept
  {
    return value;
  }

  template <typename U = T>
  ::std::enable_if_t<!::std::is_reference<U>{}, U const&>
  get() const noexcept
  {
    return value;
  }

  template <typename U = T>
  ::std::enable_if_t<::std::is_reference<U>{}, U>
  get() const noexcept
  {
    return *value;
  }

  template <typename A, typename U = T>
  ::std::enable_if_t<!::std::is_reference<U>{}>
  set(A const& u) noexcept(noexcept(::std::declval<U&>() = u))
  {
    value = u;
  }

  template <typename A, typename U = T>
  ::std::enable_if_t<!::std::is_reference<U>{}>
  set(A&& u) noexcept(noexcept(::std::declval<U&>() = ::std::move(u)))
  {
    value = ::std::move(u);
  }

  template <typename A, typename U = T>
  ::std::enable_if_t<::std::is_reference<U>{}>
  set(A&& u) noexcept
  {
    // convert to a reference to handle reference wrappers
    value = &T(u);
  }
};

template <typename, typename ...>
struct many_impl;

template <::std::size_t... Is, typename ...Types>
struct many_impl<::std::index_sequence<Is...>, Types...> :
  many_holder<Is, Types>...
{
};

}

}

template <typename ...Types>
struct many : detail::many::many_impl<
  ::std::make_index_sequence<sizeof...(Types)>,
  Types...
>
{
};

namespace detail
{

namespace many
{

template <typename ...Types, typename ...A, ::std::size_t ...Is>
auto create_many_impl(::std::index_sequence<Is...>, A&& ...a) noexcept(
  noexcept(
    (
      static_cast<
        ::generic::detail::many::many_holder<Is,
          ::std::tuple_element_t<Is, ::generic::many<Types...>>
        >&
      >(::std::declval<::generic::many<Types...>&>()).set(
        ::std::forward<A>(a)
      ),
      ...
    )
  )
)
{
  ::generic::many<Types...> m;

  (
    static_cast<
      ::generic::detail::many::many_holder<Is,
        ::std::tuple_element_t<Is, ::generic::many<Types...>>
      >&
    >(m).set(::std::forward<A>(a)),
    ...
  );

  return m;
}

}

}

template <typename ...Types, typename ...A>
auto create_many(A&& ...a) noexcept(
  noexcept(
    detail::many::create_many_impl<Types...>(
      ::std::index_sequence_for<Types...>{},
      ::std::forward<A>(a)...
    )
  )
)
{
  return detail::many::create_many_impl<Types...>(
    ::std::index_sequence_for<Types...>{},
    ::std::forward<A>(a)...
  );
}

}

namespace std
{

template <typename ...Types>
struct tuple_size<::generic::many<Types...>> :
  ::std::integral_constant<::std::size_t, sizeof...(Types)> 
{
};

template <::std::size_t I, class... Types>
struct tuple_element<I, ::generic::many<Types...>>
{
  using type = tuple_element_t<I, ::std::tuple<Types...>>;
};

template<::std::size_t I, typename... Types> 
auto& get(::generic::many<Types...>& m) noexcept
{
  return static_cast<
    ::generic::detail::many::many_holder<I,
      ::std::tuple_element_t<I, ::generic::many<Types...>>
    >&
  >(m).get();
}

template<size_t I, typename... Types> 
auto& get(::generic::many<Types...> const& m) noexcept
{
  return static_cast<
    ::generic::detail::many::many_holder<I,
      ::std::tuple_element_t<I, ::generic::many<Types...>>
    >const&
  >(m).get();
}

template<::std::size_t I, typename... Types> 
auto&& get(::generic::many<Types...>&& m) noexcept
{
  return static_cast<
    ::generic::detail::many::many_holder<I,
      ::std::tuple_element_t<I, ::generic::many<Types...>>
    >&&
  >(m).get();
}

}
