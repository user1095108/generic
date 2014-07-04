#ifndef FORWARDER_HPP
# define FORWARDER_HPP
# pragma once

#include <cassert>

#include <cstddef>

#include <cstdint>

#include <type_traits>

#include <utility>

namespace generic
{

template<typename F>
class forwarder;

template<typename R, typename ...A>
class forwarder<R (A...)>
{
  static constexpr auto max_functor_size = 3 * sizeof(::std::uintptr_t);

  template <typename U>
  static R invoker_stub(void const* const ptr, A&&... args)
  {
    return (*static_cast<U const*>(ptr))(::std::forward<A>(args)...);
  }

  R (*stub_)(void const*, A&&...){};

  typename ::std::aligned_storage<max_functor_size>::type store_;

public:
  forwarder() = default;

  forwarder(forwarder const&) = default;

  template<typename T> forwarder(T&& f) { *this = ::std::forward<T>(f); }

  forwarder& operator=(forwarder const&) = default;

  forwarder& operator=(forwarder&&) = default;

  template <
    typename T,
    typename = typename ::std::enable_if<
      !::std::is_same<forwarder, typename ::std::decay<T>::type>{}
    >::type
  >
  forwarder& operator=(T&& f)
  {
    static_assert(sizeof(T) <= sizeof(store_),
      "functor too large");
    static_assert(::std::is_trivially_destructible<T>::value,
      "functor not trivially destructible");
//  static_assert(::std::is_trivially_copy_assignable<T>::value,
//    "functor not trivially copy assignable");
//  static_assert(::std::is_trivially_copy_constructible<T>::value,
//    "functor not trivially copy constructible");

    using functor_type = typename ::std::decay<T>::type;
    new (static_cast<void*>(&store_)) functor_type(::std::forward<T>(f));

    stub_ = invoker_stub<functor_type>;

    return *this;
  }

  explicit operator bool() const noexcept { return stub_; }

  void reset() noexcept { stub_ = {}; }

  R operator()(A... args) const
  {
    //assert(stub_);
    return stub_(&store_, ::std::forward<A>(args)...);
  }
};

}

#endif // FORWARDER_HPP
