#pragma once
#ifndef VARIANT_HPP
# define VARIANT_HPP

#include <cassert>

#include <array>

#include <exception>

#include <utility>

#include <type_traits>

#include <typeinfo>

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
      + (index_of<A, C...>{} == -1 ? -1 : 1+index_of<A, C...>{})
    >
{
};

template <typename A, typename B>
struct index_of<A, B>
  : std::integral_constant < int, std::is_same<A, B>{} -1 >
{
};

template <typename A, typename B, typename... C>
struct compatible_index_of
  : std::integral_constant<int,
      std::is_constructible<A, B>{}
      + (compatible_index_of<A, C...>{} == -1
        ? -1
        : 1 + compatible_index_of<A, C...>{})
    >
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
  typedef typename std::conditional<std::is_constructible<A, B>{}, A,
    typename compatible_type<A, C...>::type>::type type;
};

template <typename A, typename B>
struct compatible_type<A, B>
{
  typedef typename std::conditional<
    std::is_constructible<A, B>{}, A, void>::type type;
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
struct one_of : bool_<A::value || one_of<B...>::value> { };

template <class A>
struct one_of<A> : bool_<A::value> { };

}

template <typename ...T>
struct variant
{
  static_assert(!::detail::one_of<std::is_reference<T>...>::value,
    "reference types are unsupported");
  static_assert(::detail::all_of<std::is_move_constructible<T>...>::value,
    "unmovable types are unsupported");

  static constexpr auto const max_align = detail::max_align<T...>::align;

  typedef typename detail::max_type<T...>::type max_type;

  variant() = default;

  ~variant()
  {
    if (-1 != store_type_)
    {
      deleter_(store_);
    }
    // else do nothing
  }

  variant(variant const&) = delete;

  variant(variant&& other)
  {
    *this = std::move(other);
  }

  variant& operator=(variant const&) = delete;

  variant& operator=(variant&& rhs)
  {
    if (-1 != rhs.store_type_)
    {
      rhs.mover_(rhs, *this);

      deleter_ = rhs.deleter_;

      mover_ = rhs.mover_;

      store_type_ = rhs.store_type_;

      rhs.store_type_ = -1;
    }
    // else do nothing

    return *this;
  }

  template <
    typename U,
    typename = typename std::enable_if<
      ::detail::one_of<std::is_same<T,
        typename std::remove_const<U>::type>...
      >::value
    >::type
  >
  variant(U&& f)
  {
    *this = std::forward<U>(f);
  }

  template <
    typename U,
    typename = typename std::enable_if<
        ::detail::one_of<std::is_same<
          typename std::remove_const<T>::type, U>...
      >::value
    >::type
  >
  variant& operator=(U&& f)
  {
    if (-1 != store_type_)
    {
      deleter_(store_);
    }
    // else do nothing

    new (store_) U(std::forward<U>(f));

    deleter_ = destructor_stub<U>;

    mover_ = mover_stub<U>;

    store_type_ = ::detail::index_of<U,
      typename std::remove_const<T>::type...>::value;

    return *this;
  }

  template <typename U>
  bool contains() const
  {
    return (store_type_ >= 0)
      && (::detail::index_of<U,
        typename std::remove_const<T>::type...>::value == store_type_);
  }

  template <typename U,
    typename = typename std::enable_if<
      -1 != ::detail::index_of<U,
        typename std::remove_const<T>::type...>::value
    >::type
  >
  U& get() const
  {
    if (::detail::index_of<U,
      typename std::remove_const<T>::type...>::value == store_type_)
    {
      return *(const_cast<U*>(static_cast<U const*>(
        static_cast<void const*>(store_))));
    }
    else
    {
      throw std::bad_typeid();
    }
  }

  template <typename U>
  U get(typename std::enable_if<(-1 ==
    ::detail::index_of<U, typename std::remove_const<T>::type...>::value)
    && (-1 != ::detail::compatible_index_of<U,
      typename std::remove_const<T>::type...>::value)
    && std::is_arithmetic<U>::value
    && std::is_arithmetic<typename ::detail::compatible_type<U, T...>::type>
      ::value>::type* = nullptr) const
  {
    if (::detail::compatible_index_of<U,
      typename std::remove_const<T>::type...>::value == store_type_)
    {
      return *static_cast<typename ::detail::compatible_type<U, T...>
        ::type const*>(static_cast<void const*>(store_));
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
  static void mover_stub(variant& src, variant& dst)
  {
    new (dst.store_) U(std::move(*static_cast<U*>(
      static_cast<void*>(src.store_))));

    src.deleter_(src.store_);
  }

  alignas(max_align) char store_[sizeof(max_type)];

  int store_type_{ -1 };

  typedef void (*deleter_type)(void*);
  deleter_type deleter_;

  typedef void (*mover_type)(variant&, variant&);
  mover_type mover_;
};

#endif // VARIANT_HPP
