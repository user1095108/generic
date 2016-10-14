#ifndef GENERIC_COROUTINE_HPP
# define GENERIC_COROUTINE_HPP
# pragma once

#include <cassert>

#include <csetjmp>

#include <functional>

#include "forwarder.hpp"

#include "lightptr.hpp"

namespace generic
{

class coroutine
{
  jmp_buf env_in_;
  jmp_buf env_out_;

  ::std::function<void()> f_;

  bool running_;
  bool terminated_;

  ::generic::light_ptr<char[]> stack_;

  char* const stack_top_;

public:
  explicit coroutine(::std::size_t const N = 128 * 1024) :
    running_{false},
    terminated_{true},
    stack_(new char[N]),
    stack_top_(stack_.get() + N)
  {
  }

  template <typename F>
  explicit coroutine(::std::size_t const N, F&& f) :
    coroutine(N)
  {
    assign(::std::forward<F>(f));
  }

  auto terminated() const noexcept
  {
    return terminated_;
  }

  template <typename F>
  void assign(F&& f)
  {
    running_ = terminated_ = false;

    f_ = [this, f = ::std::forward<F>(f)]() mutable 
      {
        f(*this);

        running_ = false;

        terminated_ = true;

        yield();
      };
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
    else if (running_)
    {
      longjmp(env_out_, 1);
    }
    else
    {
      running_ = true;

      // stack switch
#if defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
      asm volatile(
        "movq %%rsp, %0"
        :
        : "rm" (stack_top_)
      );
#elif defined(i386) || defined(__i386) || defined(__i386__)
      asm volatile(
        "movl %%esp, %0"
        :
        : "rm" (stack_top_)
      );
#else
#error "can't switch stack frame"
#endif

      f_();
    }
  }
};

}

#endif // GENERIC_COROUTINE_HPP
