#ifndef GENERIC_VARIANT_HPP
# define GENERIC_VARIANT_HPP
# pragma once

#include <cassert>

#include <ostream>

#include <type_traits>

#include <typeinfo>

#include <utility>

namespace generic
{

namespace detail
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
    (alignof(A) > alignof(B)), A, B
  >::type;
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

template <template <typename> class, typename, typename = void>
struct is_comparable : ::std::false_type { };

template <template <typename> class R, typename U>
struct is_comparable<R,
  U,
  decltype(void(::std::declval<R<U> const&>()(
    ::std::declval<U const&>(), ::std::declval<U const&>())))
> : ::std::true_type
{
};

template <typename, typename = void>
struct is_hashable : ::std::false_type { };

template <typename U>
struct is_hashable<U,
  decltype(void(::std::hash<U>()(::std::declval<U const&>())))
> : ::std::true_type
{
};

template <class, class, typename = void>
struct is_streamable : ::std::false_type { };

template <class S, class C>
struct is_streamable<S, C,
  decltype(void(::std::declval<S&>() << ::std::declval<C const&>()))
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

template <typename ...T>
class variant
{
  static_assert(!detail::any_of< ::std::is_reference<T>...>{},
    "reference types are unsupported");
  static_assert(!detail::any_of< ::std::is_void<T>...>{},
    "void type is unsupported");
  static_assert(detail::all_of<
    detail::is_move_or_copy_constructible<T>...>{},
    "unmovable and uncopyable types are unsupported");
  static_assert(!detail::has_duplicates<T...>{},
    "duplicate types are unsupported");

  using max_align_type = typename detail::max_align_type<T...>::type;

  using max_size_type = typename detail::max_size_type<T...>::type;

  static constexpr auto const max_align = alignof(max_align_type);

  template <typename ...U>
  friend class variant;

  template <int I, typename U>
  int convert_type_index() const noexcept
  {
    return I == type_index_ ? type_index<U>() : -1;
  }

  template <int I, typename U, typename ...V>
  typename ::std::enable_if<bool(sizeof...(V)), int>::type
  convert_type_index() const noexcept
  {
    return I == type_index_ ?
      type_index<U>() :
      convert_type_index<I + 1, V...>();
  }

  template <int I, typename U>
  typename ::std::enable_if<!detail::is_hashable<U>{}, ::std::size_t>::type
  hash() const
  {
    throw ::std::bad_typeid();
  }

  template <int I, typename U>
  typename ::std::enable_if<detail::is_hashable<U>{}, ::std::size_t>::type
  hash() const
  {
    return I == type_index_ ?
      ::std::hash<U>(get<U>()) :
      throw ::std::bad_typeid();
  }

  template <int I, typename U, typename ...V>
  typename ::std::enable_if<bool(sizeof...(V)) &&
    !detail::is_hashable<U>{}, int>::type
  hash() const
  {
    return I == type_index_ ? throw ::std::bad_typeid() : hash<I + 1, V...>();
  }

  template <int I, typename U, typename ...V>
  typename ::std::enable_if<bool(sizeof...(V)) &&
    detail::is_hashable<U>{}, int>::type
  hash() const
  {
    return I == type_index_ ? ::std::hash<U>(get<U>()) : hash<I + 1, V...>();
  }

/*
  template <template <typename> class R, int I, typename U, typename A>
  bool binary_relation(A const& a) const noexcept
  {
    return detail::index_of<U, T...>{} == type_index_ ?
      R<U>()(get<U>(), a) :
      false;
  }

  template <template <typename> class R, typename U, typename ...V,
    typename A>
  typename ::std::enable_if<bool(sizeof...(V)), bool>::type
  binary_relation(A const& a) const noexcept
  {
    return detail::index_of<U, T...>{} == type_index_ ?
      R<U>()(get<U>(), a) :
      binary_relation<R, V...>(a);
  }
*/

  template <template <typename> class R, int I, typename U, typename ...A>
  typename ::std::enable_if<!detail::is_comparable<R, U>{}, bool>::type
  binary_relation(variant<A...> const& a) const noexcept
  {
    throw ::std::bad_typeid();
  }

  template <template <typename> class R, int I, typename U, typename ...A>
  typename ::std::enable_if<detail::is_comparable<R, U>{}, bool>::type
  binary_relation(variant<A...> const& a) const noexcept
  {
    return I == type_index_ ?
      R<U>()(get<U>(), a.template get<U>()) :
      throw ::std::bad_typeid();
  }

  template <template <typename> class R, int I, typename U, typename ...V,
    typename ...A>
  typename ::std::enable_if<bool(sizeof...(V)) &&
    !detail::is_comparable<R, U>{}, bool>::type
  binary_relation(variant<A...> const& a) const noexcept
  {
    return I == type_index_ ?
      throw ::std::bad_typeid() :
      binary_relation<R, I + 1, V...>(a);
  }

  template <template <typename> class R, int I, typename U, typename ...V,
    typename ...A>
  typename ::std::enable_if<bool(sizeof...(V)) &&
    detail::is_comparable<R, U>{}, bool>::type
  binary_relation(variant<A...> const& a) const noexcept
  {
    return I == type_index_ ?
      R<U>()(get<U>(), a.template get<U>()) :
      binary_relation<R, I + 1, V...>(a);
  }

  template <::std::size_t I, typename OS, typename U>
  typename ::std::enable_if<!detail::is_streamable<OS, U>{}, OS&>::type
  stream_value(OS& os) const
  {
    throw ::std::bad_typeid();
  }

  template <::std::size_t I, typename OS, typename U>
  typename ::std::enable_if<detail::is_streamable<OS, U>{}, OS&>::type
  stream_value(OS& os) const
  {
    return I == type_index_ ? os << get<U>() : os;
  }

  template <::std::size_t I, typename OS, typename U, typename ...V>
  typename ::std::enable_if<!detail::is_streamable<OS, U>{} &&
    bool(sizeof...(V)), OS&>::type
  stream_value(OS& os) const
  {
    return I == type_index_ ?
      throw ::std::bad_typeid() :
      stream_value<I + 1, OS, V...>(os);
  }

  template <::std::size_t I, typename OS, typename U, typename ...V>
  typename ::std::enable_if<detail::is_streamable<OS, U>{} &&
    bool(sizeof...(V)), OS&>::type
  stream_value(OS& os) const
  {
    return I == type_index_ ?
      os << get<U>() :
      stream_value<I + 1, OS, V...>(os);
  }

public:
  variant() = default;

  ~variant() { deleter_(&store_); }

  variant(variant const& other) { *this = other; }

  variant(variant&& other) { *this = ::std::move(other); }

  bool operator==(variant const& v) const noexcept
  {
    return (v.type_index_ == type_index_) && *this ?
      binary_relation<::std::equal_to, 0, T...>(v) :
      type_index_ == v.type_index_;
  }

  template <typename ...U>
  bool operator==(variant<U...> const& v) const noexcept
  {
    auto const converted_type_index(v.template convert_type_index<0, T...>());

    return (converted_type_index == type_index_) && *this ?
      binary_relation<::std::equal_to, 0, T...>(v) :
      converted_type_index == type_index_;
  }

  bool operator<(variant const& v) const noexcept
  {
    return (v.type_index_ == type_index_) && *this ?
      binary_relation<::std::less, 0, T...>(v) :
      type_index_ < v.type_index_;
  }

  template <typename ...U>
  bool operator<(variant<U...> const& v) const noexcept
  {
    auto const converted_type_index(v.template convert_type_index<0, T...>());

    return (converted_type_index == type_index_) && *this?
      binary_relation<::std::less, 0, T...>(v) :
      type_index_ < converted_type_index;
  }

  bool operator<=(variant const& v) const noexcept
  {
    return (v.type_index_ == type_index_) && *this ?
      binary_relation<::std::less_equal, 0, T...>(v) :
      type_index_ <= v.type_index_;
  }

  template <typename ...U>
  bool operator<=(variant<U...> const& v) const noexcept
  {
    auto const converted_type_index(v.template convert_type_index<0, T...>());

    return (converted_type_index == type_index_) && *this ?
      binary_relation<::std::less_equal, 0, T...>(v) :
      type_index_ <= converted_type_index;
  }

  bool operator>(variant const& v) const noexcept
  {
    return (v.type_index_ == type_index_) && *this ?
      binary_relation<::std::greater, 0, T...>(v) :
      type_index_ > v.type_index_;
  }

  template <typename ...U>
  bool operator>(variant<U...> const& v) const noexcept
  {
    auto const converted_type_index(v.template convert_type_index<0, T...>());

    return converted_type_index == type_index_ ?
      binary_relation<::std::greater, 0, T...>(v) :
      type_index_ > converted_type_index;
  }

  bool operator>=(variant const& v) const noexcept
  {
    return v.type_index_ == type_index_ ?
      binary_relation<::std::greater_equal, 0, T...>(v) :
      type_index_ >= v.type_index_;
  }

  template <typename ...U>
  bool operator>=(variant<U...> const& v) const noexcept
  {
    auto const converted_type_index(v.template convert_type_index<0, T...>());

    return converted_type_index == type_index_ ?
      binary_relation<::std::greater_equal, 0, T...>(v) :
      type_index_ >= converted_type_index;
  }

  variant& operator=(variant const& rhs)
  {
    if (!rhs)
    {
      clear();
    }
    else if (rhs.copier_)
    {
      rhs.copier_(type_index_ == rhs.type_index_, deleter_,
        store_, rhs.store_);

      deleter_ = rhs.deleter_;
      copier_ = rhs.copier_;
      mover_ = rhs.mover_;

      type_index_ = rhs.type_index_;
    }
    else
    {
      throw ::std::bad_typeid();
    }

    return *this;
  }

  template <typename ...U>
  variant& operator=(variant<U...> const& rhs)
  {
    if (!rhs)
    {
      clear();
    }
    else if (rhs.copier_)
    {
      auto const converted_type_index(
        rhs.template convert_type_index<0, T...>()
      );

      if (-1 == converted_type_index)
      {
        throw ::std::bad_typeid();
      }
      else
      {
        rhs.copier_(type_index_ == converted_type_index, deleter_,
          store_, rhs.store_);

        deleter_ = rhs.deleter_;
        copier_ = rhs.copier_;
        mover_ = rhs.mover_;

        type_index_ = converted_type_index;
      }
    }
    else
    {
      throw ::std::bad_typeid();
    }

    return *this;
  }

  variant& operator=(variant&& rhs)
  {
    if (!rhs)
    {
      clear();
    }
    else if (rhs.mover_)
    {
      rhs.mover_(type_index_ == rhs.type_index_, deleter_,
        store_, rhs.store_);

      deleter_ = rhs.deleter_;
      copier_ = rhs.copier_;
      mover_ = rhs.mover_;

      type_index_ = rhs.type_index_;
    }
    else
    {
      throw ::std::bad_typeid();
    }

    return *this;
  }

  template <typename ...U>
  variant& operator=(variant<U...>&& rhs)
  {
    if (!rhs)
    {
      clear();
    }
    else if (rhs.mover_)
    {
      auto const converted_type_index(
        rhs.template convert_type_index<0, T...>()
      );

      if (-1 == converted_type_index)
      {
        throw ::std::bad_typeid();
      }
      else
      {
        rhs.mover_(type_index_ == converted_type_index, deleter_,
          store_, rhs.store_);

        deleter_ = rhs.deleter_;
        copier_ = rhs.copier_;
        mover_ = rhs.mover_;

        type_index_ = converted_type_index;
      }
    }
    else
    {
      throw ::std::bad_typeid();
    }

    return *this;
  }

  template <
    typename U,
    typename = typename ::std::enable_if<detail::any_of< ::std::is_same<
      typename ::std::decay<U>::type, T>...>{} &&
      !::std::is_same<typename ::std::decay<U>::type, variant>{}
    >::type
  >
  variant(U&& f)
  {
    *this = ::std::forward<U>(f);
  }

  template <typename U>
  typename ::std::enable_if<detail::any_of< ::std::is_same<
      typename ::std::decay<U>::type, T>...>{} &&
    ::std::is_copy_assignable<typename ::std::decay<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& u)
  {
    using user_type = typename ::std::decay<U>::type;

    if (detail::index_of<user_type, T...>{} == type_index_)
    {
      *reinterpret_cast<user_type*>(store_) = u;
    }
    else
    {
      clear();

      new (static_cast<void*>(store_)) user_type(::std::forward<U>(u));

      deleter_ = deleter_stub<user_type>;
      copier_ = get_copier<user_type>();
      mover_ = get_mover<user_type>();

      type_index_ = detail::index_of<user_type, T...>{};
    }

    return *this;
  }

  template <typename U>
  typename ::std::enable_if<
    detail::any_of< ::std::is_same<
      typename ::std::decay<U>::type, T>...>{} &&
    ::std::is_rvalue_reference<U&&>{} &&
    !::std::is_copy_assignable<typename ::std::decay<U>::type>{} &&
    ::std::is_move_assignable<typename ::std::decay<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& u)
  {
    using user_type = typename ::std::decay<U>::type;

    if (detail::index_of<user_type, T...>{} == type_index_)
    {
      *reinterpret_cast<user_type*>(store_) = ::std::move(u);
    }
    else
    {
      clear();

      new (static_cast<void*>(store_)) user_type(::std::forward<U>(u));

      deleter_ = deleter_stub<user_type>;
      copier_ = get_copier<user_type>();
      mover_ = get_mover<user_type>();

      type_index_ = detail::index_of<user_type, T...>{};
    }

    return *this;
  }

  template <typename U>
  typename ::std::enable_if<
    detail::any_of< ::std::is_same<
      typename ::std::decay<U>::type, T>...>{} &&
    !::std::is_copy_assignable<
      typename ::std::decay<U>::type>{} &&
    !::std::is_move_assignable<
      typename ::std::decay<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& u)
  {
    using user_type = typename ::std::decay<U>::type;

    clear();

    new (static_cast<void*>(store_)) user_type(::std::forward<U>(u));

    deleter_ = deleter_stub<user_type>;
    copier_ = get_copier<user_type>();
    mover_ = get_mover<user_type>();

    type_index_ = detail::index_of<user_type, T...>{};

    return *this;
  }

  explicit operator bool() const noexcept { return -1 != type_index_; }

  template <typename U>
  variant& assign(U&& u)
  {
    return operator=(::std::forward<U>(u));
  }

  template <typename U>
  bool contains() const noexcept
  {
    return detail::index_of<U, T...>{} == type_index_;
  }

  void clear()
  { 
    deleter_(store_);

    deleter_ = dummy_deleter_stub;
    copier_ = nullptr;
    mover_ = nullptr;

    type_index_ = -1;
  }

  bool empty() const noexcept { return !*this; }

  void swap(variant& other)
  {
    if (-1 == other.type_index_)
    {
      if (mover_)
      {
        other = ::std::move(*this);
        clear();
      }
      else if (copier_)
      {
        other = *this;
        clear();
      }
      // else do nothing
    }
    else if (-1 == type_index_)
    {
      if (other.mover_)
      {
        *this = ::std::move(other);
        other.clear();
      }
      else if (other.copier_)
      {
        *this = other;
        other.clear();
      }
      // else do nothing
    }
    else if (mover_ && other.mover_)
    {
      variant tmp(::std::move(other));

      other = ::std::move(*this);
      *this = ::std::move(tmp);
    }
    else if (mover_ && other.copier_)
    {
      variant tmp(other);

      other = ::std::move(*this);
      *this = tmp;
    }
    else if (copier_ && other.mover_)
    {
      variant tmp(::std::move(other));

      other = *this;
      *this = ::std::move(tmp);
    }
    else if (copier_ && other.copier_)
    {
      variant tmp(other);

      other = *this;
      *this = tmp;
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  cget() const
  {
    return get<U>();
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 == detail::index_of<U, T...>{}) &&
    (-1 != detail::compatible_index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  cget() const
  {
    return get<U>();
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::index_of<U, T...>{}) &&
    !(::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U const&
  >::type
  cget() const
  {
    return get<U>();
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::index_of<U, T...>{}),
    U&
  >::type
  get()
  {
    if (detail::index_of<U, T...>{} == type_index_)
    {
      return *reinterpret_cast<U*>(store_);
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::index_of<U, T...>{}),
    U const&
  >::type
  get() const
  {
    if (detail::index_of<U, T...>{} == type_index_)
    {
      return *reinterpret_cast<U const*>(store_);
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 == detail::index_of<U, T...>{}) &&
    (-1 != detail::compatible_index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  get() const
  {
    static_assert(::std::is_same<
      typename detail::type_at<
        detail::compatible_index_of<U, T...>{}, T...>::type,
      typename detail::compatible_type<U, T...>::type>{},
      "internal error");
    if (detail::compatible_index_of<U, T...>{} == type_index_)
    {
      return U(*reinterpret_cast<
        typename detail::compatible_type<U, T...>::type const*>(store_));
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  static constexpr int type_index() noexcept
  {
    return detail::index_of<U, T...>{};
  }

  int type_index() const noexcept { return type_index_; }

private:
  using deleter_type = void (*)(void*);
  using copier_type = void (*)(bool, deleter_type, void*, void const*);
  using mover_type = void (*)(bool, deleter_type, void*, void*);

  template <typename charT, typename traits>
  friend ::std::basic_ostream<charT, traits>& operator<<(
    ::std::basic_ostream<charT, traits>& os, variant const& v)
  {
    return -1 == v.type_index_ ?
      os << "<empty variant>" :
      v.stream_value<0, decltype(os), T...>(os);
  }

  template <class U>
  typename ::std::enable_if<
    ::std::is_copy_constructible<U>{}, copier_type
  >::type
  get_copier() const noexcept
  {
    return copier_stub<U>;
  }

  template <class U>
  typename ::std::enable_if<
    !::std::is_copy_constructible<U>{}, copier_type
  >::type
  get_copier() const noexcept
  {
    return nullptr;
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

  static void dummy_deleter_stub(void* const) noexcept { } 

  template <typename U>
  static void deleter_stub(void* const store)
  {
    reinterpret_cast<U*>(store)->~U();
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_copy_constructible<U>{} &&
    ::std::is_copy_assignable<U>{}
  >::type
  copier_stub(bool const same_type, deleter_type const deleter,
    void* const dst_store, void const* const src_store)
  {
    if (same_type)
    {
      *reinterpret_cast<U*>(dst_store) =
        *reinterpret_cast<U const*>(src_store);
    }
    else
    {
      deleter(dst_store);

      new (dst_store) U(*reinterpret_cast<U const*>(src_store));
    }
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_copy_constructible<U>{} &&
    !::std::is_copy_assignable<U>{}
  >::type
  copier_stub(bool const, deleter_type const deleter,
    void* const dst_store, void const* const src_store)
  {
    deleter(dst_store);

    new (dst_store) U(*reinterpret_cast<U const*>(src_store));
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_move_constructible<U>{} &&
    ::std::is_move_assignable<U>{}
  >::type
  mover_stub(bool const same_type, deleter_type const deleter,
    void* const dst_store, void* const src_store)
  {
    if (same_type)
    {
      *reinterpret_cast<U*>(dst_store) =
        ::std::move(*reinterpret_cast<U*>(src_store));
    }
    else
    {
      deleter(dst_store);

      new (dst_store) U(::std::move(*reinterpret_cast<U*>(src_store)));
    }
  }

  template <typename U>
  static typename ::std::enable_if<
    ::std::is_move_constructible<U>{} &&
    !::std::is_move_assignable<U>{}
  >::type
  mover_stub(bool const, deleter_type const deleter,
    void* const dst_store, void* const src_store)
  {
    deleter(dst_store);

    new (dst_store) U(::std::move(*reinterpret_cast<U*>(src_store)));
  }

private:
  deleter_type deleter_{dummy_deleter_stub};

  copier_type copier_{};
  mover_type mover_{};

  int type_index_{-1};

  alignas(max_align_type) char store_[sizeof(max_size_type)];
};

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif // __GNUC__

}

namespace std
{
  template <typename ...T>
  struct hash<::generic::variant<T...> >
  {
    size_t operator()(::generic::variant<T...> const& v) const noexcept
    {
      auto const seed(v.hash());

      return hash<decltype(v.type_index())>()(v.type_index()) +
        0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
  };
}

#endif // GENERIC_VARIANT_HPP
