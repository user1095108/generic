#ifndef GENERIC_COROUTINE_HPP
# define GENERIC_COROUTINE_HPP
# pragma once

#include <alloca.h>

#include <cassert>

#include <csetjmp>

#include <functional>

#include "lightptr.hpp"

namespace generic
{

template <::std::size_t N = 16384>
class coroutine
{
  jmp_buf env_in_;
  jmp_buf env_out_;

  bool running_{};

  ::generic::light_ptr<char[]> stack_{new char[N]};

public:
  coroutine() = default;

  template <typename F, typename ...A>
  coroutine(F&& f, A&& ...a)
  {
    run(::std::forward<F>(f), ::std::forward<A>(a)...);
  }

  auto running() const noexcept
  {
    return running_;
  }

  template <typename F, typename ...A>
  void run(F&& f, A&& ...a)
  {
    if (setjmp(env_in_))
    {
      return;
    }
    // else do nothing

    running_ = true;

    char* top;
    top = reinterpret_cast<char*>(&top);

    alloca(top - (stack_.get() + N));

    [this, f = ::std::forward<F>(f)](A&& ...a) __attribute__ ((noinline))
    {
      f(::std::ref(*this), ::std::forward<A>(a)...);

      running_ = false;

      yield();
    }(::std::forward<A>(a)...);
  }

  void yield() noexcept
  {
    if (setjmp(env_out_))
    {
      return;
    }
    else
    {
      longjmp(env_in_, 1);
    }
  }

  void resume() noexcept
  {
    if (setjmp(env_in_))
    {
      return;
    }
    else
    {
      longjmp(env_out_, 1);
    }
  }
};

}

#endif // GENERIC_COROUTINE_HPP
