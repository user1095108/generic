#ifndef GNR_DISPATCH_HPP
# define GNR_DISPATCH_HPP
# pragma once

#include <type_traits>

#include <utility>

#include "invoke.hpp"

namespace gnr
{

namespace detail::dispatch
{

template <std::size_t I, typename ...T>
using at_t = std::tuple_element_t<I, std::tuple<T...>>;

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

template <typename F>
constexpr auto is_noexcept_dispatchable() noexcept
{
  auto const f(static_cast<std::remove_reference_t<F>*>(nullptr));

  if constexpr(std::is_void_v<decltype(F(*f)())> ||
    std::is_reference_v<decltype(F(*f)())>)
  {
    return noexcept(F(*f)());
  }
  else
  {
    return noexcept(std::declval<decltype(F(*f)())&>() = F(*f)());
  }
}

template <typename T, auto = std::is_enum_v<T>>
struct underlying_type : std::underlying_type<T> {};

template <typename T>
struct underlying_type<T, false> { using type = T; };

template <typename T>
using underlying_type_t = typename underlying_type<T>::type;

}

constexpr decltype(auto) dispatch(auto const i, auto&& ...f)
  noexcept((detail::dispatch::is_noexcept_dispatchable<decltype(f)>() && ...))
  requires(
    std::conjunction_v<
      std::is_same<
        std::decay_t<
          decltype(
            std::declval<detail::dispatch::at_t<0, decltype(f)...>>()()
          )
        >,
        std::decay_t<decltype(std::declval<decltype(f)>()())>
      >...
    >
  )
{
  using int_t =
    detail::dispatch::underlying_type_t<std::remove_const_t<decltype(i)>>;

  using R = decltype(
    std::declval<detail::dispatch::at_t<0, decltype(f)...>>()()
  );

  return [&]<auto ...I>(std::integer_sequence<int_t, I...>)
    noexcept(
      (detail::dispatch::is_noexcept_dispatchable<decltype(f)>() && ...)
    ) -> decltype(auto)
  {
    if constexpr(std::is_void_v<R>)
    {
      (void)((I == int_t(i) && (f(), true)) || ...);
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
      detail::dispatch::result_t<R> r;

      (void)((I == int_t(i) &&
        (r = reinterpret_cast<decltype(r)>(&f()))) || ...);

      return *r;
    }
    else
    {
      R r;

      (void)(((I == int_t(i)) && (r = f(), true)) || ...);

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
          detail::dispatch::is_noexcept_dispatchable<decltype(f)>();
        },
        std::forward<decltype(a)>(a)...
      )
    )
  )
#endif // __clang__
{
  using R = decltype(
    std::declval<detail::dispatch::at_t<1, decltype(a)...>>()()
  );

  if constexpr(std::is_void_v<R>)
  {
    gnr::invoke_split_cond<2>(
      [&](auto&& e, auto&& f) noexcept(noexcept(f()))
      {
        return (e == i) && (f(), true);
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
    detail::dispatch::result_t<R> r;

    gnr::invoke_split_cond<2>(
      [&](auto&& e, auto&& f) noexcept(noexcept(f()))
      {
        return (e == i) && (r = reinterpret_cast<decltype(r)>(&f()));
      },
      std::forward<decltype(a)>(a)...
    );

    return *r;
  }
  else
  {
    R r;

    gnr::invoke_split_cond<2>(
      [&](auto&& e, auto&& f) noexcept(noexcept(std::declval<R&>() = f()))
      {
        return (e == i) && (r = f(), true);
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
          decltype(std::declval<detail::dispatch::at_t<0, decltype(v)...>>())
        >,
        std::decay_t<decltype(std::declval<decltype(v)>())>
      >...
    >
  )
{
  return [&]<auto ...I>(std::index_sequence<I...>) noexcept -> decltype(auto)
  {
    detail::dispatch::result_t<detail::dispatch::at_t<0, decltype(v)...>> r{};

    (void)(
      ((I == i) && (r = reinterpret_cast<decltype(r)>(&v), true)) || ...
    );

    return *r;
  }(std::make_index_sequence<sizeof...(v)>());
}

constexpr decltype(auto) select2(auto const i, auto&& ...a) noexcept
{
  detail::dispatch::result_t<detail::dispatch::at_t<1, decltype(a)...>> r{};

  gnr::invoke_split_cond<2>(
    [&](auto&& e, auto&& v) noexcept
    {
      return (e == i) && (r = reinterpret_cast<decltype(r)>(&v), true);
    },
    std::forward<decltype(a)>(a)...
  );

  return *r;
}

}

#endif // GNR_DISPATCH_HPP
