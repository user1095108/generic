#ifndef GNR_DISPATCH_HPP
# define GNR_DISPATCH_HPP
# pragma once

#include <type_traits>

#include <utility>

namespace gnr
{

constexpr auto dispatch(auto const i, auto&& ...f)
  noexcept(noexcept((f(), ...)))
  requires(std::is_enum_v<std::remove_const_t<decltype(i)>>)
{
  using int_t = std::underlying_type_t<std::remove_cvref_t<decltype(i)>>;

  using tuple_t = std::tuple<decltype(f)...>;
  using R = decltype(std::declval<std::tuple_element_t<0, tuple_t>>()());

  return [&]<auto ...I>(std::integer_sequence<int_t, I...>)
    noexcept(noexcept((f(), ...)))
  {
    if constexpr(std::is_void_v<R>)
    {
      ((I == int_t(i) ? (f(), 0) : 0), ...);
    }
    else
    {
      R r{};

      ((I == int_t(i) ? (r = f(), 0) : 0), ...);

      return r;
    }
  }(std::make_integer_sequence<int_t, sizeof...(f)>());
}

}

#endif // GNR_DISPATCH_HPP
