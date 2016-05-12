#ifndef IS_INVOKABLE_HPP
# define IS_INVOKABLE_HPP
# pragma once

namespace generic
{

namespace
{

template <typename F, typename ...A>
auto f(int) -> decltype((void)i(::std::declval<A...>()),
  ::std::integral_constant<bool, true>{}
);

template <typename F, typename ...A>
auto f(long) -> ::std::integral_constant<bool, false>;

}

template <typename F, typename ...A>
struct is_invokable : decltype(f<F, A...>(0))
{
};

}

#endif // IS_INVOKABLE_HPP
