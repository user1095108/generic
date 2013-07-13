#pragma once
#ifndef DELEGATE_HPP
# define DELEGATE_HPP

#include <cassert>

#include <memory>

#include <new>

#include <type_traits>

#include <utility>

inline void* align(std::size_t const alignment, std::size_t const size,
  void* const ptr, std::size_t const space)
{
  auto const p(reinterpret_cast<std::size_t>(ptr));

  auto const aligned((p + alignment - 1) & -alignment);

  return space - (aligned - p) < size
    ? nullptr
    : reinterpret_cast<void *>(aligned);
}

template <typename T> class delegate;

template<class R, class ...A>
class delegate<R (A...)>
{
  template <typename U, typename = void> struct is_functora
    : std::false_type { };

  template <typename U>
  struct is_functora<U,
    typename std::enable_if<bool(sizeof((R (U::*)(A...))
      &U::operator()))>::type
  > : std::true_type { };

  template <typename U, typename = void> struct is_functorb
    : std::false_type { };

  template <typename U>
  struct is_functorb<U,
    typename std::enable_if<bool(sizeof((R (U::*)(A...) const)
      &U::operator()))>::type
  > : std::true_type { };

  template <typename U>
  struct is_functor
  {
    static constexpr bool const value = is_functora<U>::value
      || is_functorb<U>::value;
  };

  typedef R (*stub_ptr_type)(void*, A...);

  delegate(void* const o, stub_ptr_type const m)
    : object_ptr_(o),
      stub_ptr_(m)
  {
  }

public:
  constexpr delegate() { }

  delegate(delegate const&) = delete;

  delegate(delegate&&) = default;

  template <class C>
  constexpr delegate(C const* const o)
    : object_ptr_(const_cast<C*>(o))
  {
  }

  template <class C>
  constexpr delegate(C const& o)
    : object_ptr_(const_cast<C*>(&o))
  {
  }

  delegate(R (* const function_ptr)(A...))
  {
    *this = from(function_ptr);
  }

  template <class C>
  delegate(C* const object_ptr_, R (C::* const method_ptr)(A...))
  {
    *this = from(object_ptr_, method_ptr);
  }

  template <class C>
  delegate(C* const object_ptr_, R (C::* const method_ptr)(A...) const)
  {
    *this = from(object_ptr_, method_ptr);
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

  template <class C>
  delegate(R (C::* const method_ptr)(A...))
  {
    *this = from(object_ptr_, method_ptr);
  }

  template <
    typename T,
    typename = typename std::enable_if<
      !std::is_same<delegate, typename std::remove_reference<T>::type>::value
      && is_functor<typename std::remove_reference<T>::type>::value
    >::type
  >
  delegate(T&& f)
    : stub_ptr_(functor_stub<typename std::remove_reference<T>::type>),
      functor_deleter_(
        functor_deleter<typename std::remove_reference<T>::type>),
      store_size_(sizeof(typename std::remove_reference<T>::type)),
      store_(object_ptr_ = operator new(
        sizeof(typename std::remove_reference<T>::type)), operator delete)
  {
    typedef typename std::remove_reference<T>::type functor_type;

    new (object_ptr_) functor_type(std::move(f));
  }

  ~delegate()
  {
    if (store_ && functor_deleter_)
    {
      functor_deleter_(store_.get());
    }
    // else do nothing
  }

  delegate& operator=(delegate const&) = delete;

  delegate& operator=(delegate&& rhs)
  {
    if (store_)
    {
      auto const tmp(functor_deleter_);
      functor_deleter_ = 0;

      tmp(store_.get());
    }
    // else do nothing

    object_ptr_ = rhs.object_ptr_;
    stub_ptr_ = rhs.stub_ptr_;

    functor_deleter_ = rhs.functor_deleter_;

    store_ = std::move(rhs.store_);
    store_size_ = rhs.store_size_;

    return *this;
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
      !std::is_same<delegate, typename std::remove_reference<T>::type>::value
      && is_functor<typename std::remove_reference<T>::type>::value
    >::type
  >
  delegate& operator=(T&& f)
  {
    typedef typename std::remove_reference<T>::type functor_type;

    if (store_)
    {
      auto const tmp(functor_deleter_);
      functor_deleter_ = 0;

      tmp(store_.get());
    }
    // else do nothing

    if (!store_.get()
      || (!(object_ptr_ = align(alignof(functor_type), sizeof(functor_type),
        store_.get(), store_size_))))
    {
      store_.reset(object_ptr_ = operator new(sizeof(functor_type)));

      store_size_ = sizeof(functor_type);
    }
    // else do nothing

    new (object_ptr_) functor_type(std::move(f));

    functor_deleter_ = functor_deleter<functor_type>;

    stub_ptr_ = functor_stub<functor_type>;

    return *this;
  }

  template <R (*function_ptr)(A...)>
  static constexpr delegate from()
  {
    return { nullptr, function_stub<function_ptr> };
  }

  template <class C, R (C::*method_ptr)(A...)>
  static constexpr delegate from(C* const object_ptr_)
  {
    return { object_ptr_, method_stub<C, method_ptr> };
  }

  template <class C, R (C::*method_ptr)(A...) const>
  static constexpr delegate from(C const* const object_ptr_)
  {
    return { const_cast<C*>(object_ptr_), const_method_stub<C, method_ptr> };
  }

  template <class C, R (C::*method_ptr)(A...)>
  static constexpr delegate from(C& object)
  {
    return { &object, method_stub<C, method_ptr> };
  }

  template <class C, R (C::*method_ptr)(A...) const>
  static constexpr delegate from(C const& object)
  {
    return { const_cast<C*>(&object), const_method_stub<C, method_ptr> };
  }

  template <typename T>
  static delegate from(T&& f)
  {
    return { std::forward<T>(f) };
  }

  static constexpr delegate from(R (* const function_ptr)(A...))
  {
    return { [function_ptr](A const... args){
      return (*function_ptr)(args...); } };
  }

  template <class C>
  static constexpr delegate from(C* const object_ptr_,
    R (C::* const method_ptr)(A...))
  {
    return { [object_ptr_, method_ptr](A const... args){
      return (object_ptr_->*method_ptr)(args...); } };
  }

  template <class C>
  static constexpr delegate from(C const* const object_ptr_,
    R (C::* const method_ptr)(A...) const)
  {
    return { [object_ptr_, method_ptr](A const... args){
      return (object_ptr_->*method_ptr)(args...); } };
  }

  template <class C>
  static constexpr delegate from(C& object,
    R (C::* const method_ptr)(A...))
  {
    return { [&object, method_ptr](A const... args){
      return (object.*method_ptr)(args...); } };
  }

  template <class C>
  static constexpr delegate from(C const& object,
    R (C::* const method_ptr)(A...) const)
  {
    return { [&object, method_ptr](A const... args){
      return (object.*method_ptr)(args...); } };
  }

  void reset() { stub_ptr_ = nullptr; }

  void swap(delegate& other)
  {
    std::swap(object_ptr_, other.object_ptr_);
    std::swap(stub_ptr_, other.stub_ptr_);
  }

  constexpr bool operator==(delegate const& rhs) const
  {
    return (stub_ptr_ == rhs.stub_ptr_)
      && (object_ptr_ == rhs.object_ptr_);
  }

  constexpr bool operator!=(delegate const& rhs) const
  {
    return !operator==(rhs);
  }

  constexpr bool operator<(delegate const& rhs) const
  {
    return (object_ptr_ < rhs.object_ptr_)
      || (stub_ptr_ < rhs.stub_ptr_);
  }

  constexpr explicit operator bool() const { return stub_ptr_; }

  template <typename ...B>
  constexpr R operator()(B&&... args) const
  {
//  assert(stub_ptr);
    return (*stub_ptr_)(object_ptr_, std::forward<B>(args)...);
  }

private:
  typedef void (*deleter_type)(void*);

  void* object_ptr_;
  stub_ptr_type stub_ptr_{ };

  deleter_type functor_deleter_;

  std::size_t store_size_;
  std::unique_ptr<void, deleter_type> store_{ nullptr, operator delete };

  template <class T>
  static void functor_deleter(void* const p)
  {
    static_cast<T*>(p)->~T();
  }

  template <R (*function_ptr)(A...)>
  static constexpr R function_stub(void* const,
    A const... args)
  {
    return (*function_ptr)(args...);
  }

  template <class C, R (C::*method_ptr)(A...)>
  static constexpr R method_stub(void* const object_ptr_,
    A const... args)
  {
    return (static_cast<C*>(object_ptr_)->*method_ptr)(args...);
  }

  template <class C, R (C::*method_ptr)(A...) const>
  static constexpr R const_method_stub(void* const object_ptr_,
    A const... args)
  {
    return (static_cast<C const*>(object_ptr_)->*method_ptr)(args...);
  }

  template <typename T>
  static constexpr R functor_stub(void* const object_ptr_,
    A const... args)
  {
    return (*static_cast<T*>(object_ptr_))(args...);
  }
};

#endif // DELEGATE_HPP
