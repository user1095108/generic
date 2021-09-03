#ifndef GNR_INVOKE_HPP
# define GNR_INVOKE_HPP
# pragma once

#include <tuple>

namespace gnr
{

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

template <std::size_t N>
constexpr void invoke_split(auto f, auto&& ...a)
{
  constexpr auto split([](auto&& t) noexcept requires (bool(N))
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
  );

  std::apply([&](auto&& ...t) noexcept(noexcept(
    (std::apply(f, std::forward<decltype(t)>(t)), ...)))
    {
      (std::apply(f, std::forward<decltype(t)>(t)), ...);
    },
    split(std::forward_as_tuple(a...))
  );
}

}

#endif // GNR_INVOKE_HPP
