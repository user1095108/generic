#pragma once
#ifndef DELEGATE_HPP
# define DELEGATE_HPP

#include <cassert>

#include <cstdlib>

#include <memory>

#include <new>

#include <type_traits>

#include <utility>

#include "lightptr.hpp"

namespace
{
  template <typename T, typename A = unsigned>
  struct static_store
  {
    static constexpr auto const max_instances = 8 * sizeof(A);

    static void cleanup() { delete [] store_; }

#ifdef __GNUC__
    template <typename U>
    static int ffz(U v)
    {
      return __builtin_ctzll(~v);
    }
#elif _MSC_VER && !__INTEL_COMPILER
    template <typename U>
    static int ffz(U v)
    {
      return 8 * sizeof(v) - __lzcnt64(v & -v);
    }
#elif __INTEL_COMPILER
    template <typename U>
    static int ffz(U v)
    {
      return _bit_scan_forward(~v);
    }
#else
    template <typename U>
    static int ffz(U v)
    {
      decltype(ffz()) b{};

      for (; (v & 1); ++b)
      {
        v >>= 1;
      }

      return b;
    }
#endif // __GNUC__

    static A memory_map_;
    static typename ::std::aligned_storage<sizeof(T),
      alignof(T)>::type* store_;
  };

  template <typename T, typename A>
  A static_store<T, A>::memory_map_;

  template <typename T, typename A>
  typename ::std::aligned_storage<sizeof(T), alignof(T)>::type*
    static_store<T, A>::store_{(::std::atexit(static_store<T>::cleanup),
      new typename ::std::aligned_storage<sizeof(T),
        alignof(T)>::type[static_store<T>::max_instances])};

  template <typename T, typename ...A>
  inline T* static_new(A&& ...args)
  {
    using static_store = static_store<T>;

    auto const i(static_store::ffz(static_store::memory_map_));

    auto p(new (&static_store::store_[i]) T(::std::forward<A>(args)...));

    static_store::memory_map_ |= 1 << i;

    return p;
  }

  template <typename T>
  inline void static_delete(T const* const p)
  {
    using static_store = static_store<T>;

    auto const i(p - static_cast<T const*>(static_cast<void const*>(
      static_store::store_)));
    //assert(!as_const(static_store::memory_map_)[i]);

    static_store::memory_map_ &= ~(1 << i);

    static_cast<T const*>(static_cast<void const*>(
      &static_store::store_[i]))->~T();
  }
}

template <typename T> class delegate;

template<class R, class ...A>
class delegate<R (A...)>
{
  using stub_ptr_type = R (*)(void*, A&&...);

  delegate(void* const o, stub_ptr_type const m) noexcept
    : object_ptr_(o),
      stub_ptr_(m)
  {
  }

public:
  delegate() = default;

  delegate(delegate const&) = default;

  delegate(delegate&&) = default;

  delegate(::std::nullptr_t const) noexcept : delegate() { }

  template <class C>
  explicit delegate(C const* const o) noexcept
    : object_ptr_(const_cast<C*>(o))
  {
  }

  template <class C>
  explicit delegate(C const& o) noexcept
    : object_ptr_(const_cast<C*>(&o))
  {
  }

  delegate(R (* const function_ptr)(A...))
  {
    *this = from(function_ptr);
  }

  template <class C>
  delegate(C* const object_ptr, R (C::* const method_ptr)(A...))
  {
    *this = from(object_ptr, method_ptr);
  }

  template <class C>
  delegate(C* const object_ptr, R (C::* const method_ptr)(A...) const)
  {
    *this = from(object_ptr, method_ptr);
  }

  template <class C>
  delegate(C& object, R (C::* const method_ptr)(A...))
  {
    *this = from(object, method_ptr);
  }

  template <class C>
  delegate(C const& object, R (C::* const method_ptr)(A...) const)
  {
    *this = from(object, method_ptr);
  }

  template <
    typename T,
    typename = typename ::std::enable_if<
      !::std::is_same<delegate, typename ::std::decay<T>::type>{}
    >::type
  >
  delegate(T&& f)
  {
    *this = ::std::forward<T>(f);
  }

  delegate& operator=(delegate const&) = default;

  delegate& operator=(delegate&& rhs) = default;

  delegate& operator=(R (* const rhs)(A...))
  {
    return *this = from(rhs);
  }

  template <class C>
  delegate& operator=(R (C::* const rhs)(A...))
  {
    return *this = from(static_cast<C*>(object_ptr_), rhs);
  }

  template <class C>
  delegate& operator=(R (C::* const rhs)(A...) const)
  {
    return *this = from(static_cast<C const*>(object_ptr_), rhs);
  }

  template <
    typename T,
    typename = typename ::std::enable_if<
      !::std::is_same<delegate, typename ::std::decay<T>::type>{}
    >::type
  >
  delegate& operator=(T&& f)
  {
    using functor_type = typename ::std::decay<T>::type;

    store_.reset(static_new<functor_type>(::std::forward<T>(f)),
      functor_deleter<functor_type>);

    object_ptr_ = store_.get();

    stub_ptr_ = functor_stub<functor_type>;

    return *this;
  }

  template <R (* const function_ptr)(A...)>
  static delegate from() noexcept
  {
    return { nullptr, function_stub<function_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...)>
  static delegate from(C* const object_ptr) noexcept
  {
    return { object_ptr, method_stub<C, method_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...) const>
  static delegate from(C const* const object_ptr) noexcept
  {
    return { const_cast<C*>(object_ptr), const_method_stub<C, method_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...)>
  static delegate from(C& object) noexcept
  {
    return { &object, method_stub<C, method_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...) const>
  static delegate from(C const& object) noexcept
  {
    return { const_cast<C*>(&object), const_method_stub<C, method_ptr> };
  }

  template <typename T>
  static delegate from(T&& f)
  {
    return ::std::forward<T>(f);
  }

  static delegate from(R (* const function_ptr)(A...))
  {
    return [function_ptr](A&&... args) {
      return (*function_ptr)(::std::forward<A>(args)...); };
  }

  template <class C>
  static delegate from(C* const object_ptr,
    R (C::* const method_ptr)(A...))
  {
    return [object_ptr, method_ptr](A&&... args) {
      return (object_ptr->*method_ptr)(::std::forward<A>(args)...); };
  }

  template <class C>
  static delegate from(C const* const object_ptr,
    R (C::* const method_ptr)(A...) const)
  {
    return [object_ptr, method_ptr](A&&... args) {
      return (object_ptr->*method_ptr)(::std::forward<A>(args)...); };
  }

  template <class C>
  static delegate from(C& object, R (C::* const method_ptr)(A...))
  {
    return [&object, method_ptr](A&&... args) {
      return (object.*method_ptr)(::std::forward<A>(args)...); };
  }

  template <class C>
  static delegate from(C const& object,
    R (C::* const method_ptr)(A...) const)
  {
    return [&object, method_ptr](A&&... args) {
      return (object.*method_ptr)(::std::forward<A>(args)...); };
  }

  void reset() { stub_ptr_ = nullptr; store_.reset(); }

  void reset_stub() noexcept { stub_ptr_ = nullptr; }

  void swap(delegate& other) noexcept { ::std::swap(*this, other); }

  bool operator==(delegate const& rhs) const noexcept
  {
    return (object_ptr_ == rhs.object_ptr_) && (stub_ptr_ == rhs.stub_ptr_);
  }

  bool operator!=(delegate const& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  bool operator<(delegate const& rhs) const noexcept
  {
    return (object_ptr_ < rhs.object_ptr_) ||
      ((object_ptr_ == rhs.object_ptr_) && (stub_ptr_ < rhs.stub_ptr_));
  }

  bool operator==(::std::nullptr_t const) const noexcept
  {
    return !stub_ptr_;
  }

  bool operator!=(::std::nullptr_t const) const noexcept
  {
    return stub_ptr_;
  }

  explicit operator bool() const noexcept { return stub_ptr_; }

  R operator()(A... args) const
  {
//  assert(stub_ptr);
    return stub_ptr_(object_ptr_, ::std::forward<A>(args)...);
  }

private:
  friend class ::std::hash<delegate>;

  void* object_ptr_;
  stub_ptr_type stub_ptr_{};

  light_ptr<void> store_;

  template <class T>
  static void functor_deleter(void* const p)
  {
    static_cast<T const*>(p)->~T();

    static_delete(static_cast<T const*>(p));
  }

  template <R (*function_ptr)(A...)>
  static R function_stub(void* const, A&&... args)
  {
    return function_ptr(::std::forward<A>(args)...);
  }

  template <class C, R (C::*method_ptr)(A...)>
  static R method_stub(void* const object_ptr, A&&... args)
  {
    return (static_cast<C*>(object_ptr)->*method_ptr)(
      ::std::forward<A>(args)...);
  }

  template <class C, R (C::*method_ptr)(A...) const>
  static R const_method_stub(void* const object_ptr, A&&... args)
  {
    return (static_cast<C const*>(object_ptr)->*method_ptr)(
      ::std::forward<A>(args)...);
  }

  template <typename T>
  static R functor_stub(void* const object_ptr, A&&... args)
  {
    return (*static_cast<T*>(object_ptr))(::std::forward<A>(args)...);
  }
};

namespace std
{
  template <typename R, typename ...A>
  struct hash<delegate<R (A...)> >
  {
    size_t operator()(delegate<R (A...)> const& d) const noexcept
    {
      auto const seed(hash<void*>()(d.object_ptr_));

      return hash<typename delegate<R (A...)>::stub_ptr_type>()(d.stub_ptr_) +
        0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
  };
}

#endif // DELEGATE_HPP
