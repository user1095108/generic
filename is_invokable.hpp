#ifndef IS_INVOKABLE_HPP
# define IS_INVOKABLE_HPP
# pragma once

namespace generic
{

namespace
{

template <typename F>
auto f(F&& i, int) -> decltype(i(), ::std::integral_constant<bool, true>{});

template <typename F>
auto f(F&& i, long) -> ::std::integral_constant<bool, false>;

}

template <typename F>
struct is_invokable : decltype(f(::std::declval<F>(), 0))
{
};

}

#endif // IS_INVOKABLE_HPP
