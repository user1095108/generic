#ifndef GNR_INVOKE_HPP
# define GNR_INVOKE_HPP
# pragma once

#include <tuple>

namespace gnr
{

namespace detail::invoke
{

constexpr bool is_nothrow_invocable(auto&& f, auto&& t) noexcept
{
  return [&]<auto ...I>(std::index_sequence<I...>)
    {
      return noexcept(
        std::invoke(
          std::forward<decltype(f)>(f),
          std::get<I>(std::forward<decltype(t)>(t))...
        )
      );
    }(std::make_index_sequence<
        std::tuple_size_v<std::remove_reference_t<decltype(t)>>
      >()
    );
}

template <std::size_t N>
constexpr auto split(auto&& t) noexcept requires(bool(N))
{
  constexpr auto n(std::tuple_size_v<std::remove_cvref_t<decltype(t)>>);
  static_assert(n && !(n % N));
  return [&]<auto ...I>(std::index_sequence<I...>) noexcept
    {
      return std::make_tuple(
        [&]<auto ...J>(std::index_sequence<J...>) noexcept
        {
          constexpr auto K(N * I);
          return std::forward_as_tuple(std::get<K + J>(t)...);
        }(std::make_index_sequence<N + I - I>())...
      );
    }(std::make_index_sequence<n / N>());
}

}

constexpr decltype(auto) apply(auto&& f, auto&& t) noexcept(
  noexcept(
    detail::invoke::is_nothrow_invocable(
      std::forward<decltype(f)>(f),
      std::forward<decltype(t)>(t)
    )
  )
)
{
  return [&]<auto ...I>(std::index_sequence<I...>)
    {
      return std::invoke(
        std::forward<decltype(f)>(f),
        std::get<I>(std::forward<decltype(t)>(t))...
      );
    }(std::make_index_sequence<
        std::tuple_size_v<std::remove_reference_t<decltype(t)>>
      >()
    );
}

template <typename F, typename ...A>
constexpr auto invoke_all(F f, A&& ...a) noexcept(noexcept(
  (f(std::forward<decltype(a)>(a)), ...)))
{
  (f(std::forward<decltype(a)>(a)), ...);
}

template <typename F, typename ...A>
constexpr auto invoke_cond(F f, A&& ...a) noexcept(noexcept(
  (f(std::forward<decltype(a)>(a)), ...)))
{
  return (f(std::forward<decltype(a)>(a)) || ...);
}

namespace detail::invoke
{

template <std::size_t N>
constexpr bool is_nothrow_split_invocable(auto f, auto&& ...a) noexcept
{
  return noexcept(
    ::gnr::apply([&](auto&& ...t) noexcept(noexcept(
      (::gnr::apply(f, std::forward<decltype(t)>(t)), ...)))
      {
        (::gnr::apply(f, std::forward<decltype(t)>(t)), ...);
      },
      detail::invoke::split<N>(std::forward_as_tuple(a...))
    )
  );
}

}

template <std::size_t N>
constexpr void invoke_split(auto f, auto&& ...a) noexcept(
  noexcept(
    detail::invoke::is_nothrow_split_invocable<N>(
      f,
      std::forward<decltype(a)>(a)...
    )
  )
)
{
  ::gnr::apply([&](auto&& ...t) noexcept(noexcept(
    (::gnr::apply(f, std::forward<decltype(t)>(t)), ...)))
    {
      (::gnr::apply(f, std::forward<decltype(t)>(t)), ...);
    },
    detail::invoke::split<N>(std::forward_as_tuple(a...))
  );
}

}

#endif // GNR_INVOKE_HPP
