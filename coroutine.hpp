#ifndef GNR_COROUTINE_HPP
# define GNR_COROUTINE_HPP
# pragma once

#include <cassert>

#include <csetjmp>

#include <cstdint>

#include <iostream>

#include <functional>

#include <memory>

#include "savestate.hpp"

namespace gnr
{

template <template <typename> class Function = std::function>
class coroutine
{
public:
  enum : std::size_t { default_stack_size = 512 * 1024 };

  enum status : std::uint8_t
  {
    INITIALIZED,
    RUNNING,
    TERMINATED
  };

private:
  statebuf env_in_;
  statebuf env_out_;

  enum status status_{TERMINATED};

  std::size_t const N_;

  std::unique_ptr<char[]> stack_;

  Function<void()> f_;

public:
  explicit coroutine(std::size_t const N = default_stack_size) :
    N_(N),
    stack_(new char[N])
  {
  }

  template <typename F>
  explicit coroutine(F&& f, std::size_t const N) :
    coroutine(N)
  {
    assign(std::forward<F>(f));
  }

  coroutine(coroutine&&) = default;

  coroutine& operator=(coroutine&&) = default;

  auto status() const noexcept
  {
    return status_;
  }

  auto is_terminated() const noexcept
  {
    return TERMINATED == status_;
  }

  template <typename F>
  void assign(F&& f)
  {
    f_ = [this, f = std::forward<F>(f)]()
      {
        status_ = RUNNING;

        f(*this);

        status_ = TERMINATED;

        yield();
      };

    status_ = INITIALIZED;
  }

#if defined(__GNUC__)
  void yield() noexcept __attribute__ ((noinline))
#elif defined(_MSC_VER)
  __declspec(noinline) void yield() noexcept
#endif
  {
#if defined(__GNUC__)
#if defined(i386) || defined(__i386) || defined(__i386__)
    asm volatile ("":::"eax", "ebx", "ecx", "edx", "esi", "edi");
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
    asm volatile ("":::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
#endif
#endif

    if (!savestate(env_out_))
    {
      restorestate(env_in_);
    }
  }

#if defined(__GNUC__)
  void resume() noexcept __attribute__ ((noinline))
#elif defined(_MSC_VER)
  __declspec(noinline) void resume() noexcept
#endif
  {
#if defined(__GNUC__)
#if defined(i386) || defined(__i386) || defined(__i386__)
    asm volatile ("":::"eax", "ebx", "ecx", "edx", "esi", "edi");
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
    asm volatile ("":::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
#endif
#endif

    assert(TERMINATED != status());
    if (savestate(env_in_))
    {
      return;
    }
    else if (RUNNING == status())
    {
      restorestate(env_out_);
    }
    else
    {
#if defined(__GNUC__)
      // stack switch
#if defined(i386) || defined(__i386) || defined(__i386__)
      asm volatile(
        "movl %0, %%esp"
        :
        : "r" (stack_.get() + N_)
      );
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
      asm volatile(
        "movq %0, %%rsp"
        :
        : "r" (stack_.get() + N_)
      );
#else
#error "can't switch stack frame"
#endif
#elif defined(_MSC_VER)
    auto const p(stack_.get() + N_);

    _asm mov esp, p
#else
#error "can't switch stack frame"
#endif
      f_();
    }
  }
};

}

#endif // GNR_COROUTINE_HPP
