#ifndef GENERIC_CIFY_HPP
# define GENERIC_CIFY_HPP
# pragma once

#include <utility>

namespace generic
{

namespace
{

template <typename F, int I, typename L, typename R, typename ...A>
F cify(L&& l, R (*)(A...))
{
  static L const l_(::std::forward<L>(l));

  struct S
  {
    static R f(A... args)
    {
      l_(::std::forward<A>(args)...);
    }
  };

  return &S::f;
}

}

template <typename F, int I = 0, typename L>
F cify(L&& l)
{
  return cify<F, I>(::std::forward<L>(l), F());
}

}

#endif // GENERIC_CIFY_HPP
