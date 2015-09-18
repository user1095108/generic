#ifndef GENERIC_FORWARDER_HPP
# define GENERIC_FORWARDER_HPP
# pragma once

#include <cassert>

#include <cstddef>

#include <cstdint>

#include <type_traits>

#include <utility>

namespace generic
{

template<typename F, ::std::size_t N = 4 * sizeof(void*)>
class forwarder;

template<typename R, typename ...A, ::std::size_t N>
class forwarder<R (A...), N>
{
  template <typename U>
  static R invoker_stub(void const* const ptr, A&&... args) noexcept(noexcept(
    (*static_cast<U const*>(ptr))(::std::forward<A>(args)...)))
  {
    return (*static_cast<U const*>(ptr))(::std::forward<A>(args)...);
  }

  R (*stub_)(void const*, A&&...){};

  typename ::std::aligned_storage<N>::type store_;

  template<typename T, typename ...U, ::std::size_t M>
  friend bool operator==(forwarder<T (U...), M> const&,
    ::std::nullptr_t) noexcept;
  template<typename T, typename ...U, ::std::size_t M>
  friend bool operator==(::std::nullptr_t,
    forwarder<T (U...), M> const&) noexcept;

  template<typename T, typename ...U, ::std::size_t M>
  friend bool operator!=(forwarder<T (U...), M> const&,
    ::std::nullptr_t) noexcept;
  template<typename T, typename ...U, ::std::size_t M>
  friend bool operator!=(::std::nullptr_t,
    forwarder<T (U...), M> const&) noexcept;

public:
  forwarder() = default;

  forwarder(forwarder const&) = default;

  template<typename T>
  forwarder(T&& f) noexcept
  {
    operator=(::std::forward<T>(f));
  }

  forwarder& operator=(forwarder const&) = default;

  forwarder& operator=(forwarder&&) = default;

  template <
    typename T,
    typename = typename ::std::enable_if<
      !::std::is_same<forwarder, typename ::std::decay<T>::type>{}
    >::type
  >
  forwarder& operator=(T&& f) noexcept
  {
    using functor_type = typename ::std::decay<T>::type;

    static_assert(sizeof(functor_type) <= sizeof(store_),
      "functor too large");
    static_assert(::std::is_trivially_copyable<T>{},
      "functor not trivially copyable");

    new (static_cast<void*>(&store_)) functor_type(::std::forward<T>(f));

    stub_ = invoker_stub<functor_type>;

    return *this;
  }

  explicit operator bool() const noexcept { return stub_; }

  R operator()(A... args) const
    noexcept(noexcept(stub_(&store_, ::std::forward<A>(args)...)))
  {
    //assert(stub_);
    return stub_(&store_, ::std::forward<A>(args)...);
  }

  template <typename T>
  void assign(T&& f) noexcept(noexcept(operator=(::std::forward<T>(f))))
  {
    operator=(::std::forward<T>(f));
  }

  void reset() noexcept { stub_ = nullptr; }

  void swap(forwarder& other) noexcept { ::std::swap(*this, other); }
};

template<typename R, typename ...A, ::std::size_t N>
bool operator==(forwarder<R (A...), N> const& f,
  ::std::nullptr_t const) noexcept
{
  return f.stub_ == nullptr;
}

template<typename R, typename ...A, ::std::size_t N>
bool operator==(::std::nullptr_t const,
  forwarder<R (A...), N> const& f) noexcept
{
  return f.stub_ == nullptr;
}

template<typename R, typename ...A, ::std::size_t N>
bool operator!=(forwarder<R (A...), N> const& f,
  ::std::nullptr_t const) noexcept
{
  return !operator==(f, nullptr);
}

template<typename R, typename ...A, ::std::size_t N>
bool operator!=(::std::nullptr_t const,
  forwarder<R (A...), N> const& f) noexcept
{
  return !operator==(f, nullptr);
}

}

#endif // GENERIC_FORWARDER_HPP
