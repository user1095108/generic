#ifndef IMPLSTORE_HPP
# define IMPLSTORE_HPP
# pragma once

#include <cstddef>

#include <new>

#include <utility>

namespace generic
{

template <class U, ::std::size_t N = 64>
class implstore
{
public:
  implstore() = default;

  template <typename ...A>
  implstore(A&& ...args)
  {
    static_assert(sizeof(U) <= sizeof(store_), "impl too large");
    new (store_) U(::std::forward<A>(args)...);

    deleter_ = deleter_stub;
  }

  ~implstore() { *this && (deleter_(*this), true); }

  implstore(implstore const&) = delete;

  implstore& operator=(implstore const&) = delete;

  U const* operator->() const noexcept
  {
    return reinterpret_cast<U*>(store_);
  }

  U* operator->() noexcept
  {
    return reinterpret_cast<U*>(store_);
  }

  U const* get() const noexcept
  {
    return reinterpret_cast<U*>(store_);
  }

  U* get() noexcept
  {
    return reinterpret_cast<U*>(store_);
  }

  U const& operator*() const noexcept
  {
    return *reinterpret_cast<U*>(store_);
  }

  U& operator*() noexcept
  {
    return *reinterpret_cast<U*>(store_);
  }

  explicit operator bool() const noexcept { return deleter_; }

private:
  static void deleter_stub(implstore& is) { is->~U(); }

private:
  void (*deleter_)(implstore&){};

  alignas(::std::max_align_t) char store_[N];
};

}

#endif // IMPLSTORE_HPP
