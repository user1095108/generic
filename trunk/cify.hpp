#ifndef GENERIC_CIFY_HPP
# define GENERIC_CIFY_HPP
# pragma once

#include <utility>

namespace generic
{

namespace
{

template <typename F, int I, typename L, typename R, typename ...A>
inline F cify(L&& l, R (*)(A...))
{
  static thread_local L l_(::std::forward<L>(l));
  static thread_local bool full;

  if (full)
  {
    l_.~L();
    new (static_cast<void*>(&l_)) L(::std::forward<L>(l));
  }
  else
  {
    full = true;
  }

  return [](A... args) noexcept(noexcept(l_(::std::forward<A>(args)...))) {
    return l_(::std::forward<A>(args)...);
  };
}

}

template <typename F, int I = 0, typename L>
inline F cify(L&& l)
{
  return cify<F, I>(::std::forward<L>(l), F());
}

}

#endif // GENERIC_CIFY_HPP
