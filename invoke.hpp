#ifndef GNR_INVOKE_HPP
# define GNR_INVOKE_HPP
# pragma once

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

}

#endif // GNR_INVOKE_HPP
