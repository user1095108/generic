#pragma once
#ifndef VARIANT_HPP
# define VARIANT_HPP

#include <cassert>

#include <type_traits>

#include <typeinfo>

#include <utility>

namespace detail
{

template <typename A, typename ...B>
struct max_align
{
  static constexpr auto const align =
    (alignof(A) > max_align<B...>::align)
      ? alignof(A)
      : max_align<B...>::align;
};

template <typename A, typename B>
struct max_align<A, B>
{
  static constexpr auto const align =
    (alignof(A) > alignof(B)) ? alignof(A) : alignof(B);
};

template <typename A>
struct max_align<A>
{
  static constexpr auto const align = alignof(A);
};

template <typename A, typename ...B>
struct max_type
{
  typedef typename std::conditional<
    (sizeof(A) > sizeof(typename max_type<B...>::type)),
    A,
    typename max_type<B...>::type
  >::type type;
};

template <typename A, typename B>
struct max_type<A, B>
{
  typedef typename std::conditional<(sizeof(A) > sizeof(B)), A, B>::type type;
};

template <typename A>
struct max_type<A>
{
  typedef A type;
};

template <typename A, typename B, typename... C>
struct index_of
  : std::integral_constant<int,
    std::is_same<A, B>{}
      ? 0
      : (-1 == index_of<A, C...>::value) ? -1 : 1 + index_of<A, C...>::value>
{
};

template <typename A, typename B>
struct index_of<A, B>
  : std::integral_constant<int, std::is_same<A, B>{} - 1>
{
};

template <typename A, typename... B>
struct has_duplicates
  : std::integral_constant<bool,
      (-1 == index_of<A, B...>::value ? has_duplicates<B...>::value : true)
    >
{
};

template <typename A>
struct has_duplicates<A>
  : std::integral_constant<bool, false>
{
};

template <typename A, typename B, typename... C>
struct compatible_index_of
  : std::integral_constant<int,
    std::is_constructible<A, B>{}
      ? 0
      : (-1 == compatible_index_of<A, C...>::value)
        ? -1
        : 1 + compatible_index_of<A, C...>::value>
{
};

template <typename A, typename B>
struct compatible_index_of<A, B>
  : std::integral_constant<int, std::is_constructible<A, B>{} - 1>
{
};

template <typename A, typename B, typename... C>
struct compatible_type
{
  typedef typename std::conditional<std::is_constructible<A, B>{}, B,
    typename compatible_type<A, C...>::type>::type type;
};

template <typename A, typename B>
struct compatible_type<A, B>
{
  typedef typename std::conditional<
    std::is_constructible<A, B>{}, B, void>::type type;
};

template <std::size_t I, typename A, typename ...B>
struct type_at : type_at<I - 1, B...>
{
};

template <typename A, typename ...B>
struct type_at<0, A, B...>
{
  typedef A type;
};

template <bool B>
using bool_ = std::integral_constant<bool, B>;

template <class A, class ...B>
struct all_of : bool_<A::value && all_of<B...>::value> { };

template <class A>
struct all_of<A> : bool_<A::value> { };

template <class A, class ...B>
struct any_of : bool_<A::value || any_of<B...>::value> { };

template <class A>
struct any_of<A> : bool_<A::value> { };

template <class A>
struct is_move_or_copy_constructible
{
  static constexpr auto const value =
    std::is_copy_constructible<A>::value
    || std::is_move_constructible<A>::value;
};

}

template <typename ...T>
struct variant
{
  static_assert(!::detail::any_of<std::is_reference<T>...>::value,
    "reference types are unsupported");
  static_assert(!::detail::any_of<std::is_void<T>...>::value,
    "void is unsupported");
  static_assert(::detail::all_of<
    ::detail::is_move_or_copy_constructible<T>...>::value,
    "unmovable and uncopyable types are unsupported");
  static_assert(!::detail::has_duplicates<
    typename std::remove_const<T>::type...>::value,
    "duplicates are unsupported");

  static constexpr auto const max_align = detail::max_align<T...>::align;

  typedef typename detail::max_type<T...>::type max_type;

  constexpr variant() = default;

  ~variant()
  {
    if (*this)
    {
      deleter_(store_);
    }
    // else do nothing
  }

  variant(variant const& other)
  {
    *this = other;
  }

  variant(variant&& other)
  {
    *this = std::move(other);
  }

  variant& operator=(variant const& rhs)
  {
    if (!rhs)
    {
      if (*this)
      {
        store_type_ = -1;

        deleter_(store_);
      }
      // else do nothing
    }
    else if (rhs.copier_)
    {
      rhs.copier_(const_cast<variant&>(rhs), *this);
    }
    else
    {
      throw std::bad_typeid();
    }

    return *this;
  }

  variant& operator=(variant&& rhs)
  {
    if (!rhs)
    {
      if (*this)
      {
        store_type_ = -1;

        deleter_(store_);
      }
      // else do nothing
    }
    else if (rhs.mover_)
    {
      rhs.mover_(rhs, *this);
    }
    else
    {
      throw std::bad_typeid();
    }

    return *this;
  }

  template <
    typename U,
    typename = typename std::enable_if<
      ::detail::any_of<std::is_same<typename std::remove_const<
        typename std::remove_reference<U>::type>::type,
        typename std::remove_const<T>::type>...
    >::value
    && !std::is_same<typename std::remove_const<
      typename std::remove_reference<U>::type>::type,
    variant>::value
    >::type
  >
  variant(U&& f)
  {
    this = std::forward<U>(f);
  }

  template <typename U>
  typename std::enable_if<
    ::detail::any_of<std::is_same<typename std::remove_const<
      typename std::remove_reference<U>::type>::type,
      typename std::remove_const<T>::type>...>::value
    && !std::is_rvalue_reference<U&&>::value
    && std::is_copy_assignable<typename std::remove_const<
      typename std::remove_reference<U>::type>::type>::value
    && !std::is_same<typename std::remove_const<
      typename std::remove_reference<U>::type>::type,
      variant>::value,
    variant&
  >::type
  operator=(U&& f)
  {
    typedef typename std::remove_const<
      typename std::remove_reference<U>::type>::type user_type;

    if (::detail::index_of<user_type,
      typename std::remove_const<T>::type...>::value == store_type_)
    {
      *static_cast<user_type*>(static_cast<void*>(store_)) = f;
    }
    else
    {
      if (*this)
      {
        deleter_(store_);
      }
      // else do nothing

      new (store_) user_type(std::forward<U>(f));

      deleter_ = destructor_stub<user_type>;

      copier_ = std::is_copy_constructible<user_type>::value
        ? copier_stub<user_type>
        : nullptr;

      mover_ = std::is_move_constructible<user_type>::value
        ? mover_stub<user_type>
        : nullptr;

      store_type_ = ::detail::index_of<user_type,
        typename std::remove_const<T>::type...>::value;
    }

    return *this;
  }

  template <typename U>
  typename std::enable_if<
    ::detail::any_of<std::is_same<typename std::remove_const<
      typename std::remove_reference<U>::type>::type,
      typename std::remove_const<T>::type>...>::value
    && std::is_rvalue_reference<U&&>::value
    && std::is_move_assignable<typename std::remove_const<
      typename std::remove_reference<U>::type>::type>::value
    && !std::is_same<typename std::remove_const<
      typename std::remove_reference<U>::type>::type,
      variant>::value,
    variant&
  >::type
  operator=(U&& f)
  {
    typedef typename std::remove_const<
      typename std::remove_reference<U>::type>::type user_type;

    if (::detail::index_of<user_type,
      typename std::remove_const<T>::type...>::value == store_type_)
    {
      *static_cast<user_type*>(static_cast<void*>(store_)) = std::move(f);
    }
    else
    {
      if (*this)
      {
        deleter_(store_);
      }
      // else do nothing

      new (store_) user_type(std::forward<U>(f));

      deleter_ = destructor_stub<user_type>;

      copier_ = std::is_copy_constructible<user_type>::value
        ? copier_stub<user_type>
        : nullptr;

      mover_ = std::is_move_constructible<user_type>::value
        ? mover_stub<user_type>
        : nullptr;

      store_type_ = ::detail::index_of<user_type,
        typename std::remove_const<T>::type...>::value;
    }

    return *this;
  }

  template <typename U>
  typename std::enable_if<
    ::detail::any_of<std::is_same<typename std::remove_const<
      typename std::remove_reference<U>::type>::type,
      typename std::remove_const<T>::type>...>::value
    && !std::is_copy_assignable<typename std::remove_const<
      typename std::remove_reference<U>::type>::type>::value
    && !std::is_move_assignable<typename std::remove_const<
      typename std::remove_reference<U>::type>::type>::value
    && !std::is_same<typename std::remove_const<
      typename std::remove_reference<U>::type>::type,
      variant>::value,
    variant&
  >::type
  operator=(U&& f)
  {
    typedef typename std::remove_const<
      typename std::remove_reference<U>::type>::type user_type;

    if (*this)
    {
      deleter_(store_);
    }
    // else do nothing

    new (store_) user_type(std::forward<U>(f));

    deleter_ = destructor_stub<user_type>;

    copier_ = std::is_copy_constructible<user_type>::value
      ? copier_stub<user_type>
      : nullptr;

    mover_ = std::is_move_constructible<user_type>::value
      ? mover_stub<user_type>
      : nullptr;

    store_type_ = ::detail::index_of<user_type,
      typename std::remove_const<T>::type...>::value;

    return *this;
  }

  constexpr explicit operator bool() const { return -1 != store_type_; }

  template <typename U>
  constexpr bool contains() const
  {
    return *this && (::detail::index_of<
      typename std::remove_const<U>::type,
        typename std::remove_const<T>::type...>::value == store_type_);
  }

  template <typename U>
  typename std::enable_if<
    (-1 != ::detail::index_of<typename std::remove_const<U>::type,
      typename std::remove_const<T>::type...>::value)
    && !std::is_enum<U>::value
    && !std::is_fundamental<U>::value,
    U&
  >::type
  get()
  {
    if (::detail::index_of<typename std::remove_const<U>::type,
      typename std::remove_const<T>::type...>::value == store_type_)
    {
      return *static_cast<U*>(static_cast<void*>(store_));
    }
    else
    {
      throw std::bad_typeid();
    }
  }

  template <typename U>
  typename std::enable_if<
    (-1 != ::detail::index_of<typename std::remove_const<U>::type,
      typename std::remove_const<T>::type...>::value)
    && !std::is_enum<U>::value
    && !std::is_fundamental<U>::value,
    U const&
  >::type
  get() const
  {
    if (::detail::index_of<typename std::remove_const<U>::type,
      typename std::remove_const<T>::type...>::value == store_type_)
    {
      return *static_cast<U const*>(static_cast<void const*>(store_));
    }
    else
    {
      throw std::bad_typeid();
    }
  }

  template <typename U>
  typename std::enable_if<(-1 !=
    ::detail::index_of<typename std::remove_const<U>::type,
      typename std::remove_const<T>::type...>::value)
    && (std::is_enum<U>::value
      || std::is_fundamental<U>::value),
    U
  >::type
  get()
  {
    if (::detail::index_of<U,
      typename std::remove_const<T>::type...>::value == store_type_)
    {
      return U(*static_cast<typename ::detail::compatible_type<U,
        typename std::remove_const<T>::type...>::type const*>(
          static_cast<void const*>(store_)));
    }
    else
    {
      throw std::bad_typeid();
    }
  }

  template <typename U>
  typename std::enable_if<(-1 ==
    ::detail::index_of<typename std::remove_const<U>::type,
      typename std::remove_const<T>::type...>::value)
    && (-1 != ::detail::compatible_index_of<
      typename std::remove_const<U>::type,
      typename std::remove_const<T>::type...>::value)
    && (std::is_enum<U>::value
      || std::is_fundamental<U>::value),
    U
  >::type
  get()
  {
    static_assert(std::is_same<
      typename ::detail::type_at<::detail::compatible_index_of<U,
        typename std::remove_const<T>::type...>::value,
        typename std::remove_const<T>::type...>::type,
      typename ::detail::compatible_type<U, T...>::type>::value,
      "internal error");
    if (::detail::compatible_index_of<U,
      typename std::remove_const<T>::type...>::value == store_type_)
    {
      return U(*static_cast<typename ::detail::compatible_type<U,
        typename std::remove_const<T>::type...>::type const*>(
          static_cast<void const*>(store_)));
    }
    else
    {
      throw std::bad_typeid();
    }
  }

private:
  template <typename U>
  static void destructor_stub(void* const p)
  {
    static_cast<U*>(p)->~U();
  }

  template <typename U>
  static typename std::enable_if<
    std::is_copy_constructible<U>::value
    && std::is_copy_assignable<U>::value
  >::type
  copier_stub(variant& src, variant& dst)
  {
    if (src.store_type_ == dst.store_type_)
    {
      *static_cast<U*>(static_cast<void*>(dst.store_)) =
        *static_cast<U*>(static_cast<void*>(src.store_));
    }
    else
    {
      if (dst)
      {
        dst.deleter_(dst.store_);
      }
      // else do nothing

      new (dst.store_) U(*static_cast<U*>(static_cast<void*>(src.store_)));

      dst.deleter_ = src.deleter_;

      dst.copier_ = src.copier_;

      dst.mover_ = src.mover_;

      dst.store_type_ = src.store_type_;
    }
  }

  template <typename U>
  static typename std::enable_if<
    std::is_copy_constructible<U>::value
    && !std::is_copy_assignable<U>::value
  >::type
  copier_stub(variant& src, variant& dst)
  {
    if (dst)
    {
      dst.deleter_(dst.store_);
    }
    // else do nothing

    new (dst.store_) U(*static_cast<U*>(static_cast<void*>(src.store_)));

    dst.deleter_ = src.deleter_;

    dst.copier_ = src.copier_;

    dst.mover_ = src.mover_;

    dst.store_type_ = src.store_type_;
  }

  template <typename U>
  static typename std::enable_if<
    !std::is_copy_constructible<U>::value
    && !std::is_copy_assignable<U>::value
  >::type
  copier_stub(variant& src, variant& dst)
  {
    assert(0);
  }

  template <typename U>
  static typename std::enable_if<
    std::is_move_constructible<U>::value
    && std::is_move_assignable<U>::value
  >::type
  mover_stub(variant& src, variant& dst)
  {
    if (src.store_type_ == dst.store_type_)
    {
      *static_cast<U*>(static_cast<void*>(dst.store_)) =
        std::move(*static_cast<U*>(static_cast<void*>(src.store_)));
    }
    else
    {
      if (dst)
      {
        dst.deleter_(dst.store_);
      }
      // else do nothing

      new (dst.store_) U(std::move(*static_cast<U*>(static_cast<void*>(
        src.store_))));

      dst.deleter_ = src.deleter_;

      dst.copier_ = src.copier_;

      dst.mover_ = src.mover_;

      dst.store_type_ = src.store_type_;
    }
  }

  template <typename U>
  static typename std::enable_if<
    std::is_move_constructible<U>::value
    && !std::is_move_assignable<U>::value
  >::type
  mover_stub(variant& src, variant& dst)
  {
    if (dst)
    {
      dst.deleter_(dst.store_);
    }
    // else do nothing

    new (dst.store_) U(std::move(*static_cast<U*>(
      static_cast<void*>(src.store_))));

    dst.deleter_ = src.deleter_;

    dst.copier_ = src.copier_;

    dst.mover_ = src.mover_;

    dst.store_type_ = src.store_type_;
  }

  template <typename U>
  static typename std::enable_if<
    !std::is_move_constructible<U>::value
    && !std::is_move_assignable<U>::value
  >::type
  mover_stub(variant& src, variant& dst)
  {
    assert(0);
  }

  typedef void (*deleter_type)(void*);
  deleter_type deleter_;

  typedef void (*mover_type)(variant&, variant&);
  mover_type copier_;
  mover_type mover_;

  int store_type_{ -1 };

  alignas(max_align) char store_[sizeof(max_type)];
};

#endif // VARIANT_HPP
