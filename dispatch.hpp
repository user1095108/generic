#ifndef GNR_DISPATCH_HPP
# define GNR_DISPATCH_HPP
# pragma once

#include <utility>

namespace gnr
{

constexpr auto dispatch(auto const i, auto&& ...f)
  noexcept(noexcept((f(), ...)))
{
  using tuple_t = std::tuple<decltype(f)...>;
  using R = decltype(std::declval<std::tuple_element_t<0, tuple_t>>()());

  return [&]<auto ...I>(std::index_sequence<I...>)
    noexcept(noexcept((f(), ...)))
  {
    if constexpr(std::is_void_v<R>)
    {
      (
        (
          I == std::underlying_type_t<std::remove_cvref_t<decltype(i)>>(i) ?
          (f(), 0) :
          0
        ),
        ...
      );
    }
    else
    {
      R r{};

      (
        (
          I == std::underlying_type_t<std::remove_cvref_t<decltype(i)>>(i) ?
          (r = f(), 0) :
          0
        ),
        ...
      );

      return r;
    }
  }(std::index_sequence_for<decltype(f)...>());
}

}

#endif // GNR_DISPATCH_HPP
