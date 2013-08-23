#pragma once
#ifndef DELEGATE_HPP
# define DELEGATE_HPP

#include <cassert>

#include <memory>

#include <new>

#include <type_traits>

#include <utility>

template <typename T> class delegate;

template<class R, class ...A>
class delegate<R (A...)>
{
  typedef R (*stub_ptr_type)(void*, A&&...);

  delegate(void* const o, stub_ptr_type const m) noexcept
    : object_ptr_(o),
      stub_ptr_(m)
  {
  }

public:
  delegate() = default;

  delegate(delegate const&) = default;

  delegate(delegate&&) = default;

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
    typename = typename std::enable_if<
      !std::is_same<delegate, typename std::decay<T>::type>{}
    >::type
  >
  delegate(T&& f)
    : store_(operator new(sizeof(T)),
        functor_deleter<typename std::decay<T>::type>),
      store_size_(sizeof(T))
  {
    typedef typename std::decay<T>::type functor_type;

    new (store_.get()) functor_type(std::forward<T>(f));

    object_ptr_ = store_.get();

    stub_ptr_ = functor_stub<functor_type>;

    deleter_ = destructor_stub<functor_type>;
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
    typename = typename std::enable_if<
      !std::is_same<delegate, typename std::decay<T>::type>{}
    >::type
  >
  delegate& operator=(T&& f)
  {
    typedef typename std::decay<T>::type functor_type;

    if ((sizeof(T) > store_size_)
      || (decltype(store_.use_count())(1) != store_.use_count()))
    {
      store_.reset(operator new(sizeof(T)), functor_deleter<functor_type>);

      store_size_ = sizeof(T);
    }
    else
    {
      deleter_(store_.get());
    }

    new (store_.get()) functor_type(std::forward<T>(f));

    object_ptr_ = store_.get();

    stub_ptr_ = functor_stub<functor_type>;

    deleter_ = destructor_stub<functor_type>;

    return *this;
  }

  template <R (* const function_ptr)(A...)>
  static delegate from() noexcept
  {
    return { nullptr, function_stub<function_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...)>
  static delegate from(C* object_ptr) noexcept
  {
    return { object_ptr, method_stub<C, method_ptr> };
  }

  template <class C, R (C::* const method_ptr)(A...) const>
  static delegate from(C const* object_ptr) noexcept
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
    return std::forward<T>(f);
  }

  static delegate from(R (* const function_ptr)(A...))
  {
    return [function_ptr](A&&... args) {
      return (*function_ptr)(std::forward<A>(args)...); };
  }

  template <class C>
  static delegate from(C* const object_ptr,
    R (C::* const method_ptr)(A...))
  {
    return [object_ptr, method_ptr](A&&... args) {
      return (object_ptr->*method_ptr)(std::forward<A>(args)...); };
  }

  template <class C>
  static delegate from(C const* const object_ptr,
    R (C::* const method_ptr)(A...) const)
  {
    return [object_ptr, method_ptr](A&&... args) {
      return (object_ptr->*method_ptr)(std::forward<A>(args)...); };
  }

  template <class C>
  static delegate from(C& object, R (C::* const method_ptr)(A...))
  {
    return [&object, method_ptr](A&&... args) {
      return (object.*method_ptr)(std::forward<A>(args)...); };
  }

  template <class C>
  static delegate from(C const& object,
    R (C::* const method_ptr)(A...) const)
  {
    return [&object, method_ptr](A&&... args) {
      return (object.*method_ptr)(std::forward<A>(args)...); };
  }

  void reset() { stub_ptr_ = nullptr; store_.reset(); }

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
    return (object_ptr_ < rhs.object_ptr_) || (stub_ptr_ < rhs.stub_ptr_);
  }

  explicit operator bool() const noexcept { return stub_ptr_; }

  R operator()(A... args) const
  {
//  assert(stub_ptr);
    return stub_ptr_(object_ptr_, std::forward<A>(args)...);
  }

private:
  friend class ::std::hash<delegate>;

  typedef void (*deleter_type)(void*);

  void* object_ptr_;
  stub_ptr_type stub_ptr_{};

  deleter_type deleter_;

  std::shared_ptr<void> store_;
  std::size_t store_size_;

  template <class T>
  static void destructor_stub(void* const p)
  {
    static_cast<T*>(p)->~T();
  }

  template <class T>
  static void functor_deleter(void* const p)
  {
    static_cast<T*>(p)->~T();

    operator delete(p);
  }

  template <R (*function_ptr)(A...)>
  static R function_stub(void* const, A&&... args)
  {
    return function_ptr(std::forward<A>(args)...);
  }

  template <class C, R (C::*method_ptr)(A...)>
  static R method_stub(void* const object_ptr, A&&... args)
  {
    return (static_cast<C*>(object_ptr)->*method_ptr)(
      std::forward<A>(args)...);
  }

  template <class C, R (C::*method_ptr)(A...) const>
  static R const_method_stub(void* const object_ptr, A&&... args)
  {
    return (static_cast<C const*>(object_ptr)->*method_ptr)(
      std::forward<A>(args)...);
  }

  template <typename T>
  static R functor_stub(void* const object_ptr, A&&... args)
  {
    return (*static_cast<T*>(object_ptr))(std::forward<A>(args)...);
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

      return hash<typename delegate<R (A...)>::stub_ptr_type>()(d.stub_ptr_)
        + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
  };
}

#endif // DELEGATE_HPP
