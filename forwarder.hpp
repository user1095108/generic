#pragma once
#ifndef FORWARDER_HPP
# define FORWARDER_HPP

#include <cassert>

#include <cstddef>

#include <cstdint>

#include <type_traits>

#include <utility>

#include <new>

namespace generic
{

template<typename F>
class forwarder;

template<typename R, typename ...A>
class forwarder<R (A...)>
{
public:
  forwarder() = default;

  forwarder(forwarder const&) = default;

  template<typename T>
  forwarder(T&& f) : stub_(invoker_stub<T>)
  {
    static_assert(sizeof(T) <= sizeof(store_),
      "functor too large");
    static_assert(::std::is_trivially_destructible<T>::value,
      "functor not trivially destructible");
    using functor_type = typename ::std::decay<T>::type;
    new (&store_) functor_type(::std::forward<T>(f));
  }

  forwarder& operator=(forwarder const&) = default;

  template <
    typename T,
    typename = typename ::std::enable_if<
      !::std::is_same<forwarder, typename ::std::decay<T>::type>{}
    >::type
  >
  forwarder& operator=(T&& f)
  {
    static_assert(sizeof(T) <= sizeof(store_), "functor too large");
    static_assert(::std::is_trivially_destructible<T>::value,
      "functor not trivially destructible");
    using functor_type = typename ::std::decay<T>::type;
    new (&store_) functor_type(::std::forward<T>(f));

    stub_ = invoker_stub<functor_type>;

    return *this;
  }

  R operator() (A... args) const
  {
    //assert(stub_);
    return stub_(&store_, ::std::forward<A>(args)...);
  }

private:
  template <typename U>
  static R invoker_stub(void const* ptr, A&&... args)
  {
    return (*static_cast<U const*>(ptr))(::std::forward<A>(args)...);
  }

  R (*stub_)(void const*, A&&...){};

  alignas(::std::max_align_t) ::std::uintptr_t store_;
};

}

#endif // FORWARDER_HPP
