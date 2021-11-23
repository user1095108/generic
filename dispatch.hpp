#ifndef GNR_DISPATCH_HPP
# define GNR_DISPATCH_HPP
# pragma once

#include <type_traits>

#include <utility>

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

}

constexpr decltype(auto) dispatch(auto const i, auto&& ...f)
  noexcept(noexcept((f(), ...)))
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
    else if constexpr(std::is_array_v<std::remove_reference_t<R>>)
    {
      std::remove_extent_t<std::remove_reference_t<R>>(*r)[];

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
    else if constexpr(std::is_reference_v<R>)
    {
      std::remove_reference_t<R>* r;

      ((I == int_t(i) ? r = &f() : nullptr), ...);

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

constexpr decltype(auto) select(auto const i, auto&& ...v) noexcept
  requires(
    std::is_integral_v<std::remove_const_t<decltype(i)>> &&
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
    if constexpr(std::is_array_v<std::remove_reference_t<R>>)
    {
      std::remove_extent_t<std::remove_reference_t<R>>(*r)[];

      ((I == i ? r = reinterpret_cast<decltype(r)>(&v) : nullptr), ...);

      return *r;
    }
    else
    {
      std::remove_reference_t<R>* r;

      ((I == i ? r = &v : nullptr), ...);

      return *r;
    }
  }(std::make_index_sequence<sizeof...(v)>());
}

}

#endif // GNR_DISPATCH_HPP
