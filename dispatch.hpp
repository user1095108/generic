#ifndef GNR_DISPATCH_HPP
# define GNR_DISPATCH_HPP
# pragma once

#include <type_traits>

#include <utility>

#include "invoke.hpp"

namespace gnr
{

namespace detail
{

template <typename A, typename ...B>
struct front
{
  using type = A;
};

template <typename ...A>
using front_t = typename front<A...>::type;

template <typename R>
using result_t = std::conditional_t<
  std::is_array_v<std::remove_reference_t<R>> &&
  (1 == std::rank_v<std::remove_reference_t<R>>) &&
  std::is_same_v<
    char,
    std::remove_const_t<std::remove_extent_t<std::remove_reference_t<R>>>
  >,
  std::remove_extent_t<std::remove_reference_t<R>>(*)[],
  std::conditional_t<
    std::is_reference_v<R>,
    std::remove_reference_t<R>*,
    R
  >
>;

constexpr void is_nothrow_dispatchable(auto&& f)
{
  if constexpr(std::is_void_v<decltype(f())> ||
    std::is_reference_v<decltype(f())>)
  {
    if (!noexcept(f())) throw;
  }
  else
  {
    if (!noexcept(std::declval<decltype(f())&>() = f())) throw;
  }
}

}

constexpr decltype(auto) dispatch(auto const i, auto&& ...f)
  noexcept(noexcept(
      (detail::is_nothrow_dispatchable(std::forward<decltype(f)>(f)), ...)
    )
  )
  requires(
    std::is_enum_v<std::remove_const_t<decltype(i)>> &&
    std::conjunction_v<
      std::is_same<
        std::decay_t<
          decltype(std::declval<detail::front_t<decltype(f)...>>()())
        >,
        std::decay_t<decltype(std::declval<decltype(f)>()())>
      >...
    >
  )
{
  using int_t = std::underlying_type_t<std::remove_const_t<decltype(i)>>;
  using R = decltype(std::declval<detail::front_t<decltype(f)...>>()());

  return [&]<auto ...I>(std::integer_sequence<int_t, I...>)
    noexcept(noexcept((f(), ...))) -> decltype(auto)
  {
    if constexpr(std::is_void_v<R>)
    {
      ((I == int_t(i) ? (f(), 0) : 0), ...);
    }
    else if constexpr(
      (
        std::is_array_v<std::remove_reference_t<R>> &&
        (1 == std::rank_v<std::remove_reference_t<R>>) &&
        std::is_same_v<
          char,
          std::remove_const_t<std::remove_extent_t<std::remove_reference_t<R>>>
        >
      ) ||
      std::is_reference_v<R>
    )
    {
      detail::result_t<R> r;

      (
        (
          I == int_t(i) ?
            r = reinterpret_cast<decltype(r)>(&f()) :
            nullptr
        ),
        ...
      );

      return *r;
    }
    else
    {
      R r;

      ((I == int_t(i) ? (r = f(), 0) : 0), ...);

      return r;
    }
  }(std::make_integer_sequence<int_t, sizeof...(f)>());
}

constexpr decltype(auto) dispatch2(auto const i, auto&& ...a)
#ifndef __clang__
  noexcept(noexcept(
      gnr::invoke_split<2>(
        [](auto&&, auto&& f)
        {
          detail::is_nothrow_dispatchable(std::forward<decltype(f)>(f));
        },
        std::forward<decltype(a)>(a)...
      )
    )
  )
#endif // __clang__
{
  using tuple_t = std::tuple<decltype(a)...>;
  using R = decltype(std::declval<std::tuple_element_t<1, tuple_t>>()());

  if constexpr(std::is_void_v<R>)
  {
    gnr::invoke_split<2>(
      [&](auto&& e, auto&& f) noexcept(noexcept(f()))
      {
        if (e == i)
        {
          f();
        }
      },
      std::forward<decltype(a)>(a)...
    );
  }
  else if constexpr(
    (
      std::is_array_v<std::remove_reference_t<R>> &&
      (1 == std::rank_v<std::remove_reference_t<R>>) &&
      std::is_same_v<
        char,
        std::remove_const_t<std::remove_extent_t<std::remove_reference_t<R>>>
      >
    ) ||
    std::is_reference_v<R>
  )
  {
    detail::result_t<R> r;

    gnr::invoke_split<2>(
      [&](auto&& e, auto&& f) noexcept(noexcept(f()))
      {
        if (e == i)
        {
          r = reinterpret_cast<decltype(r)>(&f());
        }
      },
      std::forward<decltype(a)>(a)...
    );

    return *r;
  }
  else
  {
    R r;

    gnr::invoke_split<2>(
      [&](auto&& e, auto&& f)
        noexcept(noexcept(std::declval<R&>() = f()))
      {
        if (e == i)
        {
          r = f();
        }
      },
      std::forward<decltype(a)>(a)...
    );

    return r;
  }
}

constexpr decltype(auto) select(auto const i, auto&& ...v) noexcept
  requires(
    std::conjunction_v<
      std::is_same<
        std::decay_t<
          decltype(std::declval<detail::front_t<decltype(v)...>>())
        >,
        std::decay_t<decltype(std::declval<decltype(v)>())>
      >...
    >
  )
{
  using R = detail::front_t<decltype(v)...>;

  return [&]<auto ...I>(std::index_sequence<I...>) noexcept -> decltype(auto)
  {
    detail::result_t<R> r;

    ((I == i ? r = reinterpret_cast<decltype(r)>(&v) : nullptr), ...);

    return *r;
  }(std::make_index_sequence<sizeof...(v)>());
}

constexpr decltype(auto) select2(auto const i, auto&& ...a) noexcept
{
  using tuple_t = std::tuple<decltype(a)...>;
  using R = decltype(std::declval<std::tuple_element_t<1, tuple_t>>());

  detail::result_t<R> r;

  gnr::invoke_split<2>(
    [&](auto&& e, auto&& v) noexcept
    {
      if (e == i)
      {
        r = reinterpret_cast<decltype(r)>(&v);
      }
    },
    std::forward<decltype(a)>(a)...
  );

  return *r;
}

}

#endif // GNR_DISPATCH_HPP
