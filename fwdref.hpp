#ifndef GNR_FORWARDERREF_HPP
# define GNR_FORWARDERREF_HPP
# pragma once

// std::memcpy
#include <cstring>

#include <functional>

#include <type_traits>

#include <utility>

namespace gnr
{

namespace detail::fwdref
{

template <typename, bool>
class fwdref_impl2;

template <typename R, typename ...A, bool E>
class fwdref_impl2<R (A...), E>
{
protected:
  R (*stub_)(void*, A&&...) noexcept(E) {};

  void* store_;

  template <typename F>
  static constexpr auto is_invocable() noexcept
  {
    return std::is_invocable_r_v<R, F, A...>;
  }

public:
  using result_type = R;

public:
  R operator()(A... args) const noexcept(E)
  {
    //assert(stub_);
    return stub_(store_, std::forward<A>(args)...);
  }

  template <typename F, typename = std::enable_if_t<
      std::is_invocable_r_v<R, F, A...>
    >
  >
  void assign(F&& f) noexcept
  {
    using functor_type = std::decay_t<F>;

    stub_ = [](void* const ptr, A&&... args) noexcept(E) -> R
      {
        return std::invoke(*static_cast<functor_type*>(ptr),
          std::forward<A>(args)...);
      };
    store_ = &f;
  }
};

template <typename>
class fwdref_impl;

template <typename R, typename ...A>
class fwdref_impl<R (A...)> :
  public fwdref_impl2<R (A...), false>
{
};

template <typename R, typename ...A>
class fwdref_impl<R (A...) noexcept> :
  public fwdref_impl2<R (A...), true>
{
};

}

template <typename A>
class fwdref : public detail::fwdref::fwdref_impl<A>
{
  using inherited_t = detail::fwdref::fwdref_impl<A>;

public:
  fwdref() = default;

  fwdref(fwdref const&) = default;

  fwdref(fwdref&&) = default;

  template <typename F,
    typename = std::enable_if_t<
      !std::is_same_v<std::decay_t<F>, fwdref> &&
      inherited_t::template is_invocable<F>()
    >
  >
  fwdref(F&& f) noexcept(noexcept(inherited_t::assign(std::forward<F>(f))))
  {
    inherited_t::assign(std::forward<F>(f));
  }

  fwdref& operator=(fwdref const&) = default;

  fwdref& operator=(fwdref&&) = default;

  template <typename F>
  fwdref& operator=(F&& f) noexcept(
    noexcept(inherited_t::assign(std::forward<F>(f))))
  {
    static_assert(inherited_t::template is_invocable<F>());

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

  void swap(fwdref& other) noexcept
  {
    std::swap(*this, other);
  }

  void swap(fwdref&& other) noexcept
  {
    std::swap(*this, std::move(other));
  }

  template <typename T>
  auto target() const noexcept
  {
    return reinterpret_cast<T*>(inherited_t::store_);
  }
};

}

#endif // GNR_FORWARDER_HPP
