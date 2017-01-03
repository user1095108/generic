#ifndef GNR_IMPLSTORE_HPP
# define GNR_IMPLSTORE_HPP
# pragma once

#include <cassert>

#include <cstddef>

#include <type_traits>

#include <utility>

namespace gnr
{

template <class U, std::size_t N = 64>
class implstore
{
  typename std::aligned_storage_t<N> store_;

public:
  static constexpr std::size_t const buffer_size = N;

  using value_type = U;

  template <typename ...A>
  explicit implstore(A&& ...args)
  {
    static_assert(std::is_constructible<U, A...>{}, "cannot construct U");
    static_assert(sizeof(store_) >= sizeof(U), "store_ too small");
    ::new (static_cast<void*>(&store_)) U(std::forward<A>(args)...);
  }

  ~implstore() { get()->~U(); }

  implstore(implstore const& other) : implstore(other, nullptr) { }

  implstore(implstore&& other) : implstore(std::move(other), nullptr) { }

  template <std::size_t M, typename K = U>
  implstore(implstore<U, M> const& other,
    std::enable_if_t<std::is_copy_constructible<K>{}>* = {})
  {
    ::new (static_cast<void*>(&store_)) U(*other);
  }

  template <std::size_t M, typename K = U>
  implstore(implstore<U, M>&& other,
    std::enable_if_t<std::is_move_constructible<K>{}>* = nullptr)
  {
    ::new (static_cast<void*>(&store_)) U(std::move(*other));
  }

  implstore& operator=(implstore const& rhs)
  {
    return operator=<N, U>(rhs);
  }

  template <std::size_t M, typename K = U,
    typename = std::enable_if_t<std::is_copy_assignable<K>{}>
  >
  implstore& operator=(implstore<U, M> const& rhs)
  {
    assert(this != &rhs);
    **this = *rhs;

    return *this;
  }

  implstore& operator=(implstore&& rhs)
  {
    return operator=<N, U>(std::move(rhs));
  }

  template <std::size_t M, typename K = U,
    typename = std::enable_if_t<std::is_move_assignable<K>{}>
  >
  implstore& operator=(implstore<U, M>&& rhs)
  {
    assert(this != &rhs);
    **this = std::move(*rhs);

    return *this;
  }

  auto operator->() noexcept
  {
    return reinterpret_cast<U*>(&store_);
  }

  auto operator->() const noexcept
  {
    return reinterpret_cast<U const*>(&store_);
  }

  auto& operator*() noexcept
  {
    return *reinterpret_cast<U*>(&store_);
  }

  auto& operator*() const noexcept
  {
    return *reinterpret_cast<U const*>(&store_);
  }

  auto get() noexcept
  {
    return reinterpret_cast<U*>(&store_);
  }

  auto get() const noexcept
  {
    return reinterpret_cast<U const*>(&store_);
  }
};

}

#endif // GNR_IMPLSTORE_HPP
