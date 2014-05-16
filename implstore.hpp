#ifndef IMPLSTORE_HPP
# define IMPLSTORE_HPP
# pragma once

#include <cstddef>

#include <new>

#include <type_traits>

#include <utility>

namespace generic
{

template <class U, ::std::size_t N = 64>
class implstore
{
public:
  implstore()
  {
    static_assert(sizeof(U) <= sizeof(store_),
      "impl too large");
    static_assert(::std::is_default_constructible<U>{},
      "impl is not default constructible");
    new (&store_) U;

    deleter_ = deleter_stub;
  }

  template <typename ...A>
  explicit implstore(A&& ...args)
  {
    static_assert(sizeof(U) <= sizeof(store_),
      "impl too large");
    new (&store_) U(::std::forward<A>(args)...);

    deleter_ = deleter_stub;
  }

  ~implstore() { *this && (deleter_(*this), true); }

  template <::std::size_t M>
  implstore(implstore<U, M> const& other)
  {
    static_assert(::std::is_copy_constructible<U>{},
      "impl is not copy constructible");
    new (&store_) U(*other);

    deleter_ = other.deleter_;
  }

  template <::std::size_t M>
  implstore(implstore<U, M>&& other)
  {
    static_assert(::std::is_move_constructible<U>{},
      "impl is not move constructible");

    new (&store_) U(*other);

    deleter_ = other.deleter_;
  }

  template <::std::size_t M>
  implstore& operator=(implstore<U, M> const& other)
  {
    static_assert(::std::is_copy_assignable<U>{},
      "impl is not copy assignable");

    **this = *other;

    return *this;
  }

  template <::std::size_t M>
  implstore& operator=(implstore<U, M>&& other)
  {
    static_assert(::std::is_move_assignable<U>{},
      "impl is not move assignable");

    **this = ::std::move(*other);

    return *this;
  }

  U const* operator->() const noexcept
  {
    return reinterpret_cast<U*>(&store_);
  }

  U* operator->() noexcept
  {
    return reinterpret_cast<U*>(&store_);
  }

  U const* get() const noexcept
  {
    return reinterpret_cast<U*>(&store_);
  }

  U* get() noexcept
  {
    return reinterpret_cast<U*>(&store_);
  }

  U const& operator*() const noexcept
  {
    return *reinterpret_cast<U*>(&store_);
  }

  U& operator*() noexcept
  {
    return *reinterpret_cast<U*>(&store_);
  }

  explicit operator bool() const noexcept { return deleter_; }

private:
  static void deleter_stub(implstore& is)
  {
    typedef char type_must_be_complete[sizeof(U) ? 1 : -1];
    (void)sizeof(type_must_be_complete);
    is->~U();
  }

private:
  void (*deleter_)(implstore&){};

  typename ::std::aligned_storage<N>::type store_;
};

}

#endif // IMPLSTORE_HPP
