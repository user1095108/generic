#ifndef MOVINGVARIANT_HPP
# define MOVINGVARIANT_HPP
# pragma once

#include <cassert>

#include <ostream>

#include <type_traits>

#include <typeinfo>

#include <utility>

namespace generic
{

namespace mdetail
{

template <typename A, typename ...B>
struct max_align_type
{
  using type = typename ::std::conditional<
    (alignof(A) > alignof(typename max_align_type<B...>::type)),
    A,
    typename max_align_type<B...>::type
  >::type;
};

template <typename A, typename B>
struct max_align_type<A, B>
{
  using type = typename ::std::conditional<
    (alignof(A) > alignof(B)), A, B>::type;
};

template <typename A>
struct max_align_type<A>
{
  using type = A;
};

template <typename A, typename ...B>
struct max_size_type
{
  using type = typename ::std::conditional<
    (sizeof(A) > sizeof(typename max_size_type<B...>::type)),
    A,
    typename max_size_type<B...>::type
  >::type;
};

template <typename A, typename B>
struct max_size_type<A, B>
{
  using type = typename ::std::conditional<
    (sizeof(A) > sizeof(B)), A, B>::type;
};

template <typename A>
struct max_size_type<A>
{
  using type = A;
};

template <typename A, typename B, typename ...C>
struct index_of :
  ::std::integral_constant<int,
    ::std::is_same<A, B>{} ?
    0 :
    -1 == index_of<A, C...>{} ? -1 : 1 + index_of<A, C...>{}
  >
{
};

template <typename A, typename B>
struct index_of<A, B> :
  ::std::integral_constant<int, ::std::is_same<A, B>{} - 1>
{
};

template <typename A, typename ...B>
struct has_duplicates :
  ::std::integral_constant<bool,
    -1 == index_of<A, B...>{} ? has_duplicates<B...>{} : true
  >
{
};

template <typename A>
struct has_duplicates<A> :
  ::std::integral_constant<bool, false>
{
};

template <typename A, typename B, typename ...C>
struct compatible_index_of :
  ::std::integral_constant<int,
    ::std::is_constructible<A, B>{} ?
      0 :
      -1 == compatible_index_of<A, C...>{} ?
        -1 :
        1 + compatible_index_of<A, C...>{}
  >
{
};

template <typename A, typename B>
struct compatible_index_of<A, B> :
  ::std::integral_constant<int, ::std::is_constructible<A, B>{} - 1>
{
};

template <typename A, typename B, typename ...C>
struct compatible_type
{
  using type = typename ::std::conditional<
      ::std::is_constructible<A, B>{},
      B,
      typename compatible_type<A, C...>::type
  >::type;
};

template <typename A, typename B>
struct compatible_type<A, B>
{
  using type = typename ::std::conditional<
    ::std::is_constructible<A, B>{}, B, void>::type;
};

template <class S, class C, typename = void>
struct is_streamable : ::std::false_type { };

template <class S, class C>
struct is_streamable<S,
  C,
  decltype(void(sizeof(decltype(::std::declval<S&>() <<
    ::std::declval<C const&>()))))
> : ::std::true_type
{
};

template < ::std::size_t I, typename A, typename ...B>
struct type_at : type_at<I - 1, B...>
{
};

template <typename A, typename ...B>
struct type_at<0, A, B...>
{
  using type = A;
};

template <bool B>
using bool_constant = ::std::integral_constant<bool, B>;

template <class A, class ...B>
struct all_of : bool_constant<A::value && all_of<B...>::value>
{
};

template <class A>
struct all_of<A> : bool_constant<A::value>
{
};

template <class A, class ...B>
struct any_of : bool_constant<A::value || any_of<B...>::value>
{
};

template <class A>
struct any_of<A> : bool_constant<A::value>
{
};

template <class A>
struct is_move_or_copy_constructible :
  bool_constant< ::std::is_copy_constructible<A>{} ||
    ::std::is_move_constructible<A>{}>
{
};

}

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif // __GNUC__

template <typename... T>
struct moving_variant
{
  static_assert(!mdetail::any_of< ::std::is_reference<T>...>{},
    "reference types are unsupported");
  static_assert(!mdetail::any_of< ::std::is_void<T>...>{},
    "void type is unsupported");
  static_assert(mdetail::all_of<::std::is_move_constructible<T>...>{},
    "unmovable types are unsupported");
  static_assert(!mdetail::has_duplicates<T...>{},
    "duplicate types are unsupported");

  using max_align_type = typename mdetail::max_align_type<T...>::type;

  using max_size_type = typename mdetail::max_size_type<T...>::type;

  static constexpr auto const max_align = alignof(max_align_type);

  moving_variant() = default;

  ~moving_variant() { clear(); }

  moving_variant(moving_variant const& other) = delete;

  moving_variant(moving_variant&& other) { *this = ::std::move(other); }

  moving_variant& operator=(moving_variant const& rhs) = delete;

  moving_variant& operator=(moving_variant&& rhs)
  {
    if (!rhs)
    {
      clear();
    }
    else if (rhs.mover_)
    {
      rhs.mover_(*this, rhs);
    }
    else
    {
      throw ::std::bad_typeid();
    }

    return *this;
  }

  template <
    typename U,
    typename = typename ::std::enable_if< mdetail::any_of< ::std::is_same<
      typename ::std::decay<U>::type, T>...>{} &&
      !::std::is_same<typename ::std::decay<U>::type, moving_variant>{}
    >::type
  >
  explicit moving_variant(U&& f)
  {
    *this = ::std::forward<U>(f);
  }

  template <typename S = ::std::ostream, typename U>
  typename ::std::enable_if<
    mdetail::any_of< ::std::is_same<
      typename ::std::decay<U>::type, T>...>{} &&
    ::std::is_rvalue_reference<U&&>{} &&
    ::std::is_move_assignable<typename ::std::decay<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, moving_variant>{},
    moving_variant&
  >::type
  operator=(U&& u)
  {
    using user_type = typename ::std::decay<U>::type;

    if (mdetail::index_of<user_type, T...>{} == store_type_)
    {
      *static_cast<user_type*>(static_cast<void*>(store_)) = ::std::move(u);
    }
    else
    {
      clear();

      new (store_) user_type(::std::forward<U>(u));

      deleter_ = deleter_stub<user_type>;

      mover_ = get_mover<user_type>();

      streamer_ = get_streamer<S, user_type>();

      store_type_ = mdetail::index_of<user_type, T...>{};
    }

    return *this;
  }

  template <typename S = ::std::ostream, typename U>
  typename ::std::enable_if<
    mdetail::any_of< ::std::is_same<
      typename ::std::decay<U>::type, T>...>{} &&
    ::std::is_rvalue_reference<U&&>{} &&
    !::std::is_move_assignable<
      typename ::std::decay<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, moving_variant>{},
    moving_variant&
  >::type
  operator=(U&& u)
  {
    using user_type = typename ::std::decay<U>::type;

    clear();

    new (store_) user_type(::std::forward<U>(u));

    deleter_ = deleter_stub<user_type>;

    mover_ = get_mover<user_type>();

    streamer_ = get_streamer<S, user_type>();

    store_type_ = mdetail::index_of<user_type, T...>{};

    return *this;
  }

  explicit operator bool() const noexcept { return -1 != store_type_; }

  template <typename S = ::std::ostream, typename U>
  moving_variant& assign(U&& u)
  {
    return operator=<S>(::std::forward<U>(u));
  }

  template <typename U>
  bool contains() const noexcept
  {
    return mdetail::index_of<U, T...>{} == store_type_;
  }

  void clear() { deleter_(*this); }

  bool empty() const noexcept { return !*this; }

  void swap(moving_variant& other)
  {
    if (-1 == other.store_type_)
    {
      if (mover_)
      {
        other = ::std::move(*this);
      }
      // else do nothing
    }
    else if (-1 == store_type_)
    {
      if (other.mover_)
      {
        *this = ::std::move(other);
      }
      // else do nothing
    }
    else if (mover_ && other.mover_)
    {
      moving_variant tmp(::std::move(other));

      other = ::std::move(*this);
      *this = ::std::move(tmp);
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != mdetail::index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  cget() const
  {
    if (mdetail::index_of<U, T...>{} == store_type_)
    {
      return *static_cast<U const*>(static_cast<void const*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != mdetail::index_of<U, T...>{}) &&
    !(::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U const&
  >::type
  cget() const
  {
    if (mdetail::index_of<U, T...>{} == store_type_)
    {
      return *static_cast<U const*>(static_cast<void const*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != mdetail::index_of<U, T...>{}),
    U&
  >::type
  get()
  {
    if (mdetail::index_of<U, T...>{} == store_type_)
    {
      return *static_cast<U*>(static_cast<void*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != mdetail::index_of<U, T...>{}),
    U const&
  >::type
  get() const
  {
    if (mdetail::index_of<U, T...>{} == store_type_)
    {
      return *static_cast<U const*>(static_cast<void const*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 == mdetail::index_of<U, T...>{}) &&
    (-1 != mdetail::compatible_index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  get() const
  {
    static_assert(::std::is_same<
      typename mdetail::type_at<
        mdetail::compatible_index_of<U, T...>{}, T...>::type,
      typename mdetail::compatible_type<U, T...>::type>{},
      "internal error");
    if (mdetail::compatible_index_of<U, T...>{} == store_type_)
    {
      return U(*static_cast<
        typename mdetail::compatible_type<U, T...>::type const*>(
          static_cast<void const*>(store_)));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  static constexpr int type_index() noexcept
  {
    return mdetail::index_of<U, T...>{};
  }

  int type_index() const noexcept { return store_type_; }

private:
  using mover_type = void (*)(moving_variant&, moving_variant&);
  using streamer_type = void (*)(void*, moving_variant const&);

  template <typename charT, typename traits>
  friend ::std::basic_ostream<charT, traits>& operator<<(
    ::std::basic_ostream<charT, traits>& os, moving_variant const& v)
  {
    v.streamer_(&os, v);

    return os;
  }

  template <class U>
  typename ::std::enable_if<
    ::std::is_move_constructible<U>{}, mover_type
  >::type
  get_mover() const noexcept
  {
    return mover_stub<U>;
  }

  template <class U>
  typename ::std::enable_if<
    !::std::is_move_constructible<U>{}, mover_type
  >::type
  get_mover() const noexcept
  {
    return nullptr;
  }

  template <class S, class U>
  typename ::std::enable_if<
    mdetail::is_streamable<S, U>{},
    streamer_type
  >::type
  get_streamer() const noexcept
  {
    return streamer_stub<S, U>;
  }

  template <class S, class U>
  typename ::std::enable_if<
    !mdetail::is_streamable<S, U>{},
    streamer_type
  >::type
  get_streamer() const noexcept
  {
    return nullptr;
  }

  static void dummy_deleter_stub(moving_variant&) noexcept { }

  template <typename U>
  static void deleter_stub(moving_variant& v)
  {
    v.deleter_ = dummy_deleter_stub;
    v.mover_ = nullptr;
    v.streamer_ = nullptr;

    v.store_type_ = -1;

    static_cast<U*>(static_cast<void*>(v.store_))->~U();
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_move_constructible<U>{} &&
    ::std::is_move_assignable<U>{}
  >::type
  mover_stub(moving_variant& dst, moving_variant& src)
  {
    if (src.store_type_ == dst.store_type_)
    {
      *static_cast<U*>(static_cast<void*>(dst.store_)) =
        ::std::move(*static_cast<U*>(static_cast<void*>(src.store_)));
    }
    else
    {
      dst.clear();

      new (dst.store_) U(::std::move(*static_cast<U*>(
        static_cast<void*>(src.store_))));

      dst.deleter_ = src.deleter_;

      dst.mover_ = src.mover_;

      dst.streamer_ = src.streamer_;

      dst.store_type_ = src.store_type_;
    }
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_move_constructible<U>{} &&
    !::std::is_move_assignable<U>{}
  >::type
  mover_stub(moving_variant& dst, moving_variant& src)
  {
    dst.clear();

    new (dst.store_) U(::std::move(*static_cast<U*>(
      static_cast<void*>(src.store_))));

    dst.deleter_ = src.deleter_;

    dst.mover_ = src.mover_;

    dst.streamer_ = src.streamer_;

    dst.store_type_ = src.store_type_;
  }

  template <class S, typename U>
  static typename ::std::enable_if<
    mdetail::is_streamable<S, U>{}
  >::type
  streamer_stub(void* const os, moving_variant const& v)
  {
    *static_cast<S*>(os) << v.cget<U>();
  }

  using deleter_type = void (*)(moving_variant&);
  deleter_type deleter_{dummy_deleter_stub};

  mover_type mover_;

  streamer_type streamer_;

  int store_type_{-1};

  alignas(max_align_type) char store_[sizeof(max_size_type)];
};

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif // __GNUC__

}

#endif // MOVINGVARIANT_HPP
