#ifndef GNR_INVOKE_HPP
# define GNR_INVOKE_HPP
# pragma once

#include <functional>

#include <tuple>

namespace gnr
{

namespace detail::invoke
{

constexpr void is_nothrow_invocable(auto&& f, auto&& t)
{
  [&]<auto ...I>(std::index_sequence<I...>)
  {
    if (!noexcept(
        std::invoke(
          std::forward<decltype(f)>(f),
          std::get<I>(std::forward<decltype(t)>(t))...
        )
      )
    )
    {
      throw;
    }
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
    return std::tuple(
      [&]<auto ...J>(std::index_sequence<J...>) noexcept
      {
        constexpr auto K(N * I);
        return std::forward_as_tuple(std::get<K + J>(t)...);
      }(std::make_index_sequence<N + I - I>())...
    );
  }(std::make_index_sequence<n / N>());
}

}

constexpr decltype(auto) apply(auto&& f, auto&& t)
  noexcept(noexcept(
    detail::invoke::is_nothrow_invocable(
      std::forward<decltype(f)>(f),
      std::forward<decltype(t)>(t)
    )
  )
)
{
  return [&]<auto ...I>(std::index_sequence<I...>)
    noexcept(noexcept(
      std::invoke(
        std::forward<decltype(f)>(f),
        std::get<I>(std::forward<decltype(t)>(t))...
      )
    )
  )
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

constexpr auto invoke_all(auto f, auto&& ...a)
  noexcept(noexcept((f(std::forward<decltype(a)>(a)), ...)))
{
  (f(std::forward<decltype(a)>(a)), ...);
}

constexpr auto invoke_cond(auto f, auto&& ...a)
  noexcept(noexcept((f(std::forward<decltype(a)>(a)), ...)))
{
  return (f(std::forward<decltype(a)>(a)) || ...);
}

namespace detail::invoke
{

template <std::size_t N>
constexpr void is_nothrow_split_invocable(auto&& f, auto&& ...a)
{
  if (!noexcept(
      ::gnr::apply([f](auto&& ...t) noexcept(noexcept(
        (::gnr::apply(f, std::forward<decltype(t)>(t)), ...)))
        {
          (::gnr::apply(f, std::forward<decltype(t)>(t)), ...);
        },
        detail::invoke::split<N>(std::forward_as_tuple(a...))
      )
    )
  )
  {
    throw;
  }
}

}

template <std::size_t N>
constexpr void invoke_split(auto&& f, auto&& ...a)
  noexcept(noexcept(
    detail::invoke::is_nothrow_split_invocable<N>(
      std::forward<decltype(f)>(f),
      std::forward<decltype(a)>(a)...
    )
  )
)
{
  ::gnr::apply([f](auto&& ...t) noexcept(noexcept(
    (::gnr::apply(f, std::forward<decltype(t)>(t)), ...)))
    {
      (::gnr::apply(f, std::forward<decltype(t)>(t)), ...);
    },
    detail::invoke::split<N>(std::forward_as_tuple(a...))
  );
}

}

#endif // GNR_INVOKE_HPP
