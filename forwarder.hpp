#ifndef GNR_FORWARDER_HPP
# define GNR_FORWARDER_HPP
# pragma once

#include <cassert>

// std::size_t
#include <cstddef>

#include <functional>

#include <type_traits>

#include <utility>

namespace gnr
{

namespace
{

constexpr auto const default_size = 4 * sizeof(void*);

template <typename F, typename ...A>
constexpr bool is_noexcept() noexcept
{
  return noexcept(std::declval<F>()(std::declval<A>()...));
}

}

template <typename F, std::size_t N, typename O>
class forwarder_impl;

template <typename R, typename ...A, std::size_t N, typename O>
class forwarder_impl<R (A...), N, O>
{
protected:
  R (*stub_)(void*, A&&...) noexcept(is_noexcept<O, A...>()) {};

  std::aligned_storage_t<N> store_;

public:
  using result_type = R;

public:
  R operator()(A... args) const noexcept(is_noexcept<O, A...>())
  {
    //assert(stub_);
    return stub_(const_cast<decltype(store_)*>(std::addressof(store_)),
      std::forward<A>(args)...);
  }

  template <typename F,
    typename = std::enable_if_t<std::is_invocable_r_v<R, F, A...>>
  >
  void assign(F&& f) noexcept
  {
    using functor_type = std::decay_t<F>;

    static_assert(sizeof(functor_type) <= sizeof(store_),
      "functor too large");
    static_assert(std::is_trivially_copyable<functor_type>{},
      "functor not trivially copyable");

    ::new (std::addressof(store_)) functor_type(std::forward<F>(f));

    stub_ = [](void* const ptr, A&&... args) noexcept(
      is_noexcept<O, A...>())-> R
    {
      return std::invoke(*static_cast<functor_type*>(ptr),
        std::forward<A>(args)...);
    };
  }
};

template <typename A, std::size_t N = default_size>
class forwarder : public forwarder_impl<A, N, A>
{
  using inherited_t = forwarder_impl<A, N, A>;

public:
  enum : std::size_t { size = N };

  forwarder() = default;

  forwarder(forwarder const&) = default;

  forwarder(forwarder&&) = default;

  template <typename F,
    typename = std::enable_if_t<
      !std::is_same_v<std::decay_t<F>, forwarder> &&
      std::is_invocable_v<
        decltype(&inherited_t::template assign<F>), inherited_t&, F
      >
    >
  >
  forwarder(F&& f) noexcept
  {
    inherited_t::assign(std::forward<F>(f));
  }

  forwarder& operator=(forwarder const&) = default;

  forwarder& operator=(forwarder&&) = default;

  template <typename F,
    typename = std::enable_if_t<
      !std::is_same_v<std::decay_t<F>, forwarder> &&
      std::is_invocable_v<
        decltype(&inherited_t::template assign<F>), inherited_t&, F
      >
    >
  >
  forwarder& operator=(F&& f) noexcept
  {
    inherited_t::assign(std::forward<F>(f));

    return *this;
  }

  explicit operator bool() const noexcept
  {
    return inherited_t::stub_;
  }

  bool operator==(std::nullptr_t) noexcept
  {
    return *this;
  }

  bool operator!=(std::nullptr_t) noexcept
  {
    return !operator==(nullptr);
  }

  void assign(std::nullptr_t) noexcept
  {
    reset();
  }

  void reset() noexcept
  {
    inherited_t::stub_ = {};
  }

  void swap(forwarder& other) noexcept
  {
    std::swap(*this, other);
  }

  void swap(forwarder&& other) noexcept
  {
    std::swap(*this, std::move(other));
  }

  template <typename T>
  auto target() noexcept
  {
    return reinterpret_cast<T*>(&inherited_t::store_);
  }

  template <typename T> 
  auto target() const noexcept
  {
    return reinterpret_cast<T const*>(&inherited_t::store_);
  }
};

}

#endif // GNR_FORWARDER_HPP
