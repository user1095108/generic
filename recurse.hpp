#ifndef GENERIC_RECURSE_HPP
# define GENERIC_RECURSE_HPP
# pragma once

namespace generic
{

template<class F>
auto recurse(F&& f)
{
  return [f = std::forward<F>(f)](auto&& ...args)
  {
    return f(f, decltype(args)(args)...);
  };
}

}

#endif // GENERIC_RECURSE_HPP
