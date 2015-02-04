#ifndef GENERIC_CIFY_HPP
# define GENERIC_CIFY_HPP
# pragma once

#include <utility>

namespace generic
{

namespace
{

template <typename R, typename ...A> using return_type = R (*)(A...);

}

template <typename F, int I = 0, typename R, typename ...A>
return_type<R, A...> cify(R (*)(A...), F&& f)
{
  static F const f_(::std::forward<F>(f));

  struct S
  {
    static R f(A... args)
    {
      f_(::std::forward<A>(args)...);
    }
  };

  return &S::f;
}

}

#endif // GENERIC_CIFY_HPP
