#pragma once
#ifndef VARIANT_HPP
# define VARIANT_HPP

#include <exception>

#include <array>

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

template <bool B>
using bool_ = std::integral_constant<bool, B>;

template <class A, class ...B>
struct one_of : bool_<A::value || one_of<B...>::value> { };

template <class A>
struct one_of<A> : bool_<A::value> { };

}

template <typename ...T>
struct variant
{
  static constexpr auto const max_align = detail::max_align<T...>::align;

  typedef typename detail::max_type<T...>::type max_type;

  variant() = default;

  ~variant() noexcept
  {
    if (store_type_)
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
    if (rhs.store_type_)
    {
      rhs.mover_(rhs, *this);

      deleter_ = rhs.deleter_;

      mover_ = rhs.mover_;

      store_type_ = rhs.store_type_;

      rhs.store_type_ = nullptr;
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
      ::detail::one_of<std::is_same<T,
        typename std::remove_const<U>::type>...
      >::value
    >::type
  >
  variant& operator=(U&& f)
  {
    if (store_type_)
    {
      deleter_(store_);
    }
    // else do nothing

    new (store_) U(std::forward<U>(f));

    deleter_ = destructor_stub<U>;

    mover_ = mover_stub<U>;

    store_type_ = &typeid(U);

    return *this;
  }

  template <typename U>
  bool contains() const
  {
    return store_type_ && (typeid(U) == *store_type_);
  }

  template <typename U>
  U const& get() const
  {
    if (contains<U>())
    {
      return *(static_cast<U const*>(static_cast<void const*>(store_)));
    }
    else
    {
      throw std::bad_typeid();
    }
  }

  template <typename U>
  U& get()
  {
    if (contains<U>())
    {
      return *(static_cast<U*>(static_cast<void*>(store_)));
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

  std::type_info const* store_type_{ };

  typedef void (*deleter_type)(void*);
  deleter_type deleter_;

  typedef void (*mover_type)(variant&, variant&);
  mover_type mover_;
};

#endif // VARIANT_HPP
