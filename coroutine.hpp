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

class coroutine
{
  jmp_buf env_in_;
  jmp_buf env_out_;

  bool running_{};

  ::generic::light_ptr<char[]> stack_;

  char* const stack_top_;

public:
  explicit coroutine(::std::size_t const N = 128 * 1024) :
    stack_(new char[N]),
    stack_top_(stack_.get() + N)
  {
  }

  template <typename F, typename ...A>
  explicit coroutine(::std::size_t const N, F&& f, A&& ...args) :
    coroutine(N)
  {
    run(::std::forward<F>(f), ::std::forward<A>(args)...);
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

    alloca(top - stack_top_);

    [this, f = ::std::forward<F>(f)](A&& ...a) __attribute__ ((noinline)) mutable 
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
