#pragma once
#ifndef DELEGATE_HPP
# define DELEGATE_HPP

#include <cassert>

#include <cstddef>

#include <memory>

#include <new>

#include <type_traits>

#include <utility>

template <typename T> class arraydelegate;

template<class R, class ...A>
class arraydelegate<R (A...)>
{
  static constexpr auto max_store_size = 5 * sizeof(::std::size_t);

  using stub_ptr_type = R (*)(void*, A&&...);

  arraydelegate(void* const o, stub_ptr_type const m) noexcept
    : object_ptr_(o),
      stub_ptr_(m)
  {
  }

public:
  arraydelegate() = default;

  arraydelegate(arraydelegate const& other) { *this = other; }

  arraydelegate(arraydelegate&& other) { *this = ::std::move(other); }

  arraydelegate(::std::nullptr_t const) noexcept : arraydelegate() { }

  template <class C>
  explicit arraydelegate(C const* const o) noexcept
    : object_ptr_(const_cast<C*>(o))
  {
  }

  template <class C>
  explicit arraydelegate(C const& o) noexcept
    : object_ptr_(const_cast<C*>(&o))
  {
  }

  arraydelegate(R (* const function_ptr)(A...))
  {
    *this = from(function_ptr);
  }

  template <class C>
  arraydelegate(C* const object_ptr, R (C::* const method_ptr)(A...))
  {
    *this = from(object_ptr, method_ptr);
  }

  template <class C>
  arraydelegate(C* const object_ptr, R (C::* const method_ptr)(A...) const)
  {
    *this = from(object_ptr, method_ptr);
  }

  template <class C>
  arraydelegate(C& object, R (C::* const method_ptr)(A...))
  {
    *this = from(object, method_ptr);
  }

  template <class C>
  arraydelegate(C const& object, R (C::* const method_ptr)(A...) const)
  {
    *this = from(object, method_ptr);
  }

  template <
    typename T,
    typename = typename ::std::enable_if<
      !::std::is_same<arraydelegate, typename ::std::decay<T>::type>{}
    >::type
  >
  arraydelegate(T&& f)
  {
    using functor_type = typename ::std::decay<T>::type;

    new (store_) functor_type(::std::forward<T>(f));

    object_ptr_ = store_;
    stub_ptr_ = functor_stub<functor_type>;

    deleter_ = destructor_stub<functor_type>;

    copier_ = copier_stub<functor_type>;
    mover_ = mover_stub<functor_type>;
  }

  arraydelegate& operator=(arraydelegate const& rhs)
  {
    rhs.copier_(*this, rhs);

    return *this;
  }

  arraydelegate& operator=(arraydelegate&& rhs)
  {
    rhs.mover_(*this, ::std::move(rhs));

    return *this;
  }

  arraydelegate& operator=(R (* const rhs)(A...))
  {
    return *this = from(rhs);
  }

  template <class C>
  arraydelegate& operator=(R (C::* const rhs)(A...))
  {
    return *this = from(static_cast<C*>(object_ptr_), rhs);
  }

  template <class C>
  arraydelegate& operator=(R (C::* const rhs)(A...) const)
  {
    return *this = from(static_cast<C const*>(object_ptr_), rhs);
  }

  template <
    typename T,
    typename = typename ::std::enable_if<
      !::std::is_same<arraydelegate, typename ::std::decay<T>::type>{}
    >::type
  >
  arraydelegate& operator=(T&& f)
  {
    using functor_type = typename ::std::decay<T>::type;

    if (deleter_)
    {
      deleter_(store_);

      deleter_ = nullptr;
    }
    // else do nothing

    new (store_) functor_type(::std::forward<T>(f));

    object_ptr_ = store_;
    stub_ptr_ = functor_stub<functor_type>;

    deleter_ = destructor_stub<functor_type>;

    copier_ = copier_stub<functor_type>;
    mover_ = mover_stub<functor_type>;

    return *this;
  }

  template <R (* const function_ptr)(A...)>
  static arraydelegate from() noexcept
  {
    return { nullptr, function_stub<function_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...)>
  static arraydelegate from(C* const object_ptr) noexcept
  {
    return { object_ptr, method_stub<C, method_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...) const>
  static arraydelegate from(C const* const object_ptr) noexcept
  {
    return { const_cast<C*>(object_ptr), const_method_stub<C, method_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...)>
  static arraydelegate from(C& object) noexcept
  {
    return { &object, method_stub<C, method_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...) const>
  static arraydelegate from(C const& object) noexcept
  {
    return { const_cast<C*>(&object), const_method_stub<C, method_ptr> };
  }

  template <typename T>
  static arraydelegate from(T&& f)
  {
    return ::std::forward<T>(f);
  }

  static arraydelegate from(R (* const function_ptr)(A...))
  {
    return [function_ptr](A&&... args) {
      return (*function_ptr)(::std::forward<A>(args)...); };
  }

  template <class C>
  static arraydelegate from(C* const object_ptr,
    R (C::* const method_ptr)(A...))
  {
    return [object_ptr, method_ptr](A&&... args) {
      return (object_ptr->*method_ptr)(::std::forward<A>(args)...); };
  }

  template <class C>
  static arraydelegate from(C const* const object_ptr,
    R (C::* const method_ptr)(A...) const)
  {
    return [object_ptr, method_ptr](A&&... args) {
      return (object_ptr->*method_ptr)(::std::forward<A>(args)...); };
  }

  template <class C>
  static arraydelegate from(C& object, R (C::* const method_ptr)(A...))
  {
    return [&object, method_ptr](A&&... args) {
      return (object.*method_ptr)(::std::forward<A>(args)...); };
  }

  template <class C>
  static arraydelegate from(C const& object,
    R (C::* const method_ptr)(A...) const)
  {
    return [&object, method_ptr](A&&... args) {
      return (object.*method_ptr)(::std::forward<A>(args)...); };
  }

  void reset() { stub_ptr_ = nullptr; }

  void reset_stub() noexcept { stub_ptr_ = nullptr; }

  void swap(arraydelegate& other) noexcept { ::std::swap(*this, other); }

  bool operator==(arraydelegate const& rhs) const noexcept
  {
    return (object_ptr_ == rhs.object_ptr_) && (stub_ptr_ == rhs.stub_ptr_);
  }

  bool operator!=(arraydelegate const& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  bool operator<(arraydelegate const& rhs) const noexcept
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
  friend class ::std::hash<arraydelegate>;

  using copier_type = void (*)(arraydelegate&, arraydelegate const&);

  using mover_type = void (*)(arraydelegate&, arraydelegate&&);

  using deleter_type = void (*)(void*);

  void* object_ptr_;
  stub_ptr_type stub_ptr_{};

  copier_type copier_;
  mover_type mover_;

  deleter_type deleter_;

  alignas(::max_align_t) char store_[max_store_size];

  template <class T>
  static void copier_stub(arraydelegate& dst, arraydelegate const& src)
  {
    dst.stub_ptr_ = src.stub_ptr_;

    if ((dst.deleter_ = src.deleter_))
    {
      new (dst.store_) T(*static_cast<T const*>(static_cast<void const*>(
        src.store_)));

      dst.object_ptr_ = dst.store_;

      dst.copier_ = src.copier_;
      dst.mover_= src.mover_;
    }
    else
    {
      dst.object_ptr_ = src.object_ptr_;
    }
  }

  template <class T>
  static void mover_stub(arraydelegate& dst, arraydelegate&& src)
  {
    dst.stub_ptr_ = src.stub_ptr_;

    if ((dst.deleter_ = src.deleter_))
    {
      new (dst.store_) T(::std::move(*static_cast<T*>(static_cast<void*>(
        src.store_))));

      dst.object_ptr_ = dst.store_;

      dst.copier_ = src.copier_;
      dst.mover_= src.mover_;
    }
    else
    {
      dst.object_ptr_ = src.object_ptr_;
    }
  }

  template <class T>
  static void destructor_stub(void* const p)
  {
    static_cast<T*>(p)->~T();
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
  struct hash<arraydelegate<R (A...)> >
  {
    size_t operator()(arraydelegate<R (A...)> const& d) const noexcept
    {
      auto const seed(hash<void*>()(d.object_ptr_));

      return hash<typename arraydelegate<R (A...)>::stub_ptr_type>()(
        d.stub_ptr_) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
  };
}

#endif // DELEGATE_HPP
