#ifndef IMPLSTORE_HPP
# define IMPLSTORE_HPP
# pragma once

#include <cassert>

#include <cstddef>

#include <type_traits>

#include <utility>

namespace generic
{

template <class U, ::std::size_t N = 64>
class implstore
{
public:
  static constexpr ::std::size_t const buffer_size = N;

  using value_type = U;

  template <typename ...A, typename =
    typename ::std::enable_if<::std::is_constructible<U, A...>{}>::type
  >
  implstore(A&& ...args)
  {
    static_assert(sizeof(U) <= sizeof(store_), "impl too large");
    new (static_cast<void*>(&store_)) U(::std::forward<A>(args)...);
  }

  ~implstore() { get()->~U(); }

  implstore(implstore const& other) :
    implstore(other, nullptr)
  {
  }

  implstore(implstore&& other) :
    implstore(::std::move(other), nullptr)
  {
  }

  template <::std::size_t M, typename K = U>
  implstore(implstore<U, M> const& other,
    typename ::std::enable_if<
      ::std::is_copy_constructible<K>{}
    >::type* = nullptr)
  {
    new (static_cast<void*>(&store_)) U(*other);
  }

  template <::std::size_t M, typename K = U>
  implstore(implstore<U, M>&& other, typename ::std::enable_if<
      ::std::is_move_constructible<K>{}
    >::type* = nullptr)
  {
    new (static_cast<void*>(&store_)) U(::std::move(*other));
  }

  template <::std::size_t M, typename K = U, typename =
    typename ::std::enable_if<
      ::std::is_copy_assignable<K>{}
    >::type
  >
  implstore& operator=(implstore<U, M> const& rhs)
  {
    assert(this != &rhs);
    **this = *rhs;

    return *this;
  }

  template <::std::size_t M, typename K = U, typename =
    typename ::std::enable_if<
      ::std::is_move_assignable<K>{}
    >::type
  >
  implstore& operator=(implstore<U, M>&& rhs)
  {
    assert(this != &rhs);
    **this = ::std::move(*rhs);

    return *this;
  }

  U const* operator->() const noexcept
  {
    return reinterpret_cast<U const*>(&store_);
  }

  U* operator->() noexcept
  {
    return reinterpret_cast<U*>(&store_);
  }

  U const* get() const noexcept
  {
    return reinterpret_cast<U const*>(&store_);
  }

  U* get() noexcept
  {
    return reinterpret_cast<U*>(&store_);
  }

  U const& operator*() const noexcept
  {
    return *reinterpret_cast<U const*>(&store_);
  }

  U& operator*() noexcept
  {
    return *reinterpret_cast<U*>(&store_);
  }

private:
  typename ::std::aligned_storage<N>::type store_;
};

}

#endif // IMPLSTORE_HPP
