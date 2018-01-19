#ifndef GNR_COROUTINE_HPP
# define GNR_COROUTINE_HPP
# pragma once

#include <cassert>

#include <cstdint>

#include <functional>

#include <memory>

#include "savestate.hpp"

namespace gnr
{

namespace
{

// half a megabyte stack default
enum : std::size_t { anon_default_stack_size = 512 * 1024 };

}

template <
  std::size_t N = anon_default_stack_size,
  template <typename> class Function = std::function
>
class coroutine
{
public:
  enum : std::size_t { default_stack_size = anon_default_stack_size };
  enum : std::size_t { stack_size = N };

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

  std::unique_ptr<char[]> stack_;

  Function<void()> f_;

public:
  explicit coroutine() :
    stack_(new char[stack_size])
  {
  }

  template <typename F>
  explicit coroutine(F&& f) :
    coroutine()
  {
    assign(std::forward<F>(f));
  }

  coroutine(coroutine&&) = default;

  coroutine& operator=(coroutine&&) = default;

  template <typename F>
  coroutine& operator=(F&& f)
  {
    assign(std::forward<F>(f));

    return *this;
  }

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
#else
# error "unsupported compiler"
#endif
  {
#if defined(__GNUC__)
#if defined(i386) || defined(__i386) || defined(__i386__)
    asm volatile ("":::"eax", "ebx", "ecx", "edx", "esi", "edi");
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
    asm volatile ("":::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
#elif defined(__aarch64__)
    asm volatile ("":::"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28","x29", "x30");
#elif defined(__arm__) && defined(__ARM_ARCH_7__)
    asm volatile ("":::"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "lr");
#elif defined(__arm__)
    asm volatile ("":::"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "lr");
#endif
#endif

    if (!savestate(env_out_))
    {
      restorestate(env_in_);
    }
    // else do nothing
  }

#if defined(__GNUC__)
  void resume() noexcept __attribute__ ((noinline))
#elif defined(_MSC_VER)
  __declspec(noinline) void resume() noexcept
#else
# error "unsupported compiler"
#endif
  {
#if defined(__GNUC__)
#if defined(i386) || defined(__i386) || defined(__i386__)
    asm volatile ("":::"eax", "ebx", "ecx", "edx", "esi", "edi");
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
    asm volatile ("":::"rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
#elif defined(__aarch64__)
    asm volatile ("":::"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28","x29", "x30");
#elif defined(__arm__) && defined(__ARM_ARCH_7__)
    asm volatile ("":::"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r8", "r9", "r10", "lr");
#elif defined(__arm__)
    asm volatile ("":::"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10", "lr");
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
        : "r" (stack_.get() + stack_size)
      );
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
      asm volatile(
        "movq %0, %%rsp"
        :
        : "r" (stack_.get() + stack_size)
      );
#elif defined(__arm__) || defined(__aarch64__)
      asm volatile(
        "mov sp, %0"
        :
        : "r" (stack_.get() + stack_size)
      );
#else
#error "can't switch stack frame"
#endif
#elif defined(_MSC_VER)
    auto const p(stack_.get() + stack_size);

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
