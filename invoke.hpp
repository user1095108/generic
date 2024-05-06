#ifndef GNR_INVOKE_HPP
# define GNR_INVOKE_HPP
# pragma once

#include <functional>

#include <tuple>

namespace gnr
{

namespace detail::invoke
{

template <typename>
struct is_tuple : std::false_type {};

template <typename ...T>
struct is_tuple<std::tuple<T...>> : std::true_type {};

template <typename T>
static constexpr bool is_tuple_v(is_tuple<std::remove_cvref_t<T>>{});

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
        return std::forward_as_tuple(
          std::get<K + J>(std::forward<decltype(t)>(t))...);
      }(std::make_index_sequence<N + I - I>())...
    );
  }(std::make_index_sequence<n / N>());
}

template <typename F, typename T>
constexpr bool is_noexcept_invocable() noexcept
{
  auto const f(static_cast<std::remove_reference_t<F>*>(nullptr));
  auto const t(static_cast<std::remove_reference_t<T>*>(nullptr));

  return noexcept(
    [&]<auto ...I>(std::index_sequence<I...>)
    {
      return (std::invoke(F(*f), std::get<I>(T(*t))...));
    }(std::make_index_sequence<
        std::tuple_size_v<std::remove_cvref_t<T>>
      >()
    )
  );
}

}

constexpr decltype(auto) apply(auto&& f, auto&& t)
  noexcept(
    detail::invoke::is_noexcept_invocable<decltype(f), decltype(t)>()
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
      std::tuple_size_v<std::remove_cvref_t<decltype(t)>>
    >()
  );
}

constexpr auto invoke_all(auto f, auto&& ...a)
  noexcept(noexcept((f(std::forward<decltype(a)>(a)), ...)))
{
  return (f(std::forward<decltype(a)>(a)), ...);
}

constexpr auto invoke_cond(auto f, auto&& ...a)
  noexcept(noexcept((f(std::forward<decltype(a)>(a)), ...)))
{
  return (f(std::forward<decltype(a)>(a)) || ...);
}

namespace detail::invoke
{

template <std::size_t N, typename F, typename ...A>
constexpr bool is_noexcept_split_invocable() noexcept
{
  auto const f(static_cast<std::remove_reference_t<F>*>(nullptr));

  return noexcept(
    ::gnr::apply([&](auto&& ...t)
      noexcept(noexcept(
        (::gnr::apply(F(*f), std::forward<decltype(t)>(t)), ...)))
      {
        (::gnr::apply(F(*f), std::forward<decltype(t)>(t)), ...);
      },
      detail::invoke::split<N>(
        std::forward_as_tuple(
          A(*static_cast<std::remove_reference_t<A>*>(nullptr))...
        )
      )
    )
  );
}

}

template <std::size_t N>
constexpr void invoke_split(auto&& f, auto&& ...a)
  noexcept(
    detail::invoke::is_noexcept_split_invocable<N, decltype(f),
      decltype(a)...>()
  )
{
  ::gnr::apply(
    [&](auto&& ...t)
      noexcept(noexcept(
        (::gnr::apply(f, std::forward<decltype(t)>(t)), ...))
      )
    {
      (::gnr::apply(f, std::forward<decltype(t)>(t)), ...);
    },
    detail::invoke::split<N>(std::forward_as_tuple(a...))
  );
}

template <std::size_t N>
constexpr bool invoke_split_cond(auto&& f, auto&& ...a)
  noexcept(
    detail::invoke::is_noexcept_split_invocable<
      N, decltype(f), decltype(a)...
    >()
  )
{
  return ::gnr::apply(
    [&](auto&& ...t)
      noexcept(noexcept(
          (::gnr::apply(f, std::forward<decltype(t)>(t)), ...)
        )
      )
    {
      return (::gnr::apply(f, std::forward<decltype(t)>(t)) || ...);
    },
    detail::invoke::split<N>(std::forward_as_tuple(a...))
  );
}

constexpr auto chain_apply(auto&& t, auto&& f, auto&& ...fs)
  noexcept(noexcept(
      ::gnr::apply(
        std::forward<decltype(f)>(f),
        std::forward<decltype(t)>(t)
      )
    )
  )
  requires(detail::invoke::is_tuple_v<decltype(t)>)
{
  if constexpr(sizeof...(fs))
  {
    using R = decltype(
      ::gnr::apply(
        std::forward<decltype(f)>(f),
        std::forward<decltype(t)>(t)
      )
    );

    if constexpr(gnr::detail::invoke::is_tuple_v<R>)
    {
      return chain_apply(
        ::gnr::apply(
          std::forward<decltype(f)>(f),
          std::forward<decltype(t)>(t)
        ),
        std::forward<decltype(fs)>(fs)...
      );
    }
    else if constexpr(std::is_void_v<R>)
    {
      ::gnr::apply(
        std::forward<decltype(f)>(f),
        std::forward<decltype(t)>(t)
      );

      return chain_apply(
        std::tuple(),
        std::forward<decltype(fs)>(fs)...
      );
    }
    else
    {
      return chain_apply(
        std::forward_as_tuple(
          ::gnr::apply(
            std::forward<decltype(f)>(f),
            std::forward<decltype(t)>(t)
          )
        ),
        std::forward<decltype(fs)>(fs)...
      );
    }
  }
  else
  {
    return ::gnr::apply(
      std::forward<decltype(f)>(f),
      std::forward<decltype(t)>(t)
    );
  }
}

constexpr auto chain_apply(auto&& a, auto&& ...f)
  noexcept(noexcept(
      chain_apply(
        std::forward_as_tuple(a),
        std::forward<decltype(f)>(f)...
      )
    )
  )
  requires(!detail::invoke::is_tuple_v<decltype(a)>)
{
  return chain_apply(
    std::forward_as_tuple(a),
    std::forward<decltype(f)>(f)...
  );
}

constexpr auto chain_apply(int[], auto&& ...f)
  noexcept(noexcept(
      chain_apply(
        std::tuple(),
        std::forward<decltype(f)>(f)...
      )
    )
  )
{
  return chain_apply(
    std::tuple(),
    std::forward<decltype(f)>(f)...
  );
}

}

#endif // GNR_INVOKE_HPP
