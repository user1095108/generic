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

namespace variant
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

}

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif // __GNUC__

template <typename ...T>
class variant
{
  static_assert(!detail::variant::any_of< ::std::is_reference<T>...>{},
    "reference types are unsupported");
  static_assert(!detail::variant::any_of< ::std::is_void<T>...>{},
    "void type is unsupported");
  static_assert(detail::variant::all_of<
    detail::variant::is_move_or_copy_constructible<T>...>{},
    "unmovable and uncopyable types are unsupported");
  static_assert(!detail::variant::has_duplicates<T...>{},
    "duplicate types are unsupported");

  using max_align_type = typename detail::variant::max_align_type<T...>::type;

  using max_size_type = typename detail::variant::max_size_type<T...>::type;

  static constexpr auto const max_align = alignof(max_align_type);

  template <typename ...U>
  friend class variant;

  friend struct ::std::hash<variant<T...> >;

  template <int I, typename U>
  int convert_type_id() const noexcept
  {
    return I == type_id_ ? type_id<U>() : -1;
  }

  template <int I, typename U, typename ...V>
  typename ::std::enable_if<bool(sizeof...(V)), int>::type
  convert_type_id() const noexcept
  {
    return I == type_id_ ? type_id<U>() : convert_type_id<I + 1, V...>();
  }

  template <int I, typename U>
  typename ::std::enable_if<!detail::variant::is_hashable<U>{},
    ::std::size_t>::type
  get_hash() const
  {
    throw ::std::bad_typeid();
  }

  template <int I, typename U>
  typename ::std::enable_if<detail::variant::is_hashable<U>{},
    ::std::size_t>::type
  get_hash() const
  {
    return I == type_id_ ?
      ::std::hash<U>()(get<U>()) :
      throw ::std::bad_typeid();
  }

  template <int I, typename U, typename ...V>
  typename ::std::enable_if<
    bool(sizeof...(V)) && !detail::variant::is_hashable<U>{},
    int
  >::type
  get_hash() const
  {
    return I == type_id_ ?
      throw ::std::bad_typeid() :
      get_hash<I + 1, V...>();
  }

  template <int I, typename U, typename ...V>
  typename ::std::enable_if<
    bool(sizeof...(V)) && detail::variant::is_hashable<U>{},
    int
  >::type
  get_hash() const
  {
    return I == type_id_ ?
      ::std::hash<U>()(get<U>()) :
      get_hash<I + 1, V...>();
  }

  template <template <typename> class R, int I, typename U, typename ...A>
  typename ::std::enable_if<
    !detail::variant::is_comparable<R, U>{}, bool
  >::type
  binary_relation(variant<A...> const& a) const noexcept
  {
    throw ::std::bad_typeid();
  }

  template <template <typename> class R, int I, typename U, typename ...A>
  typename ::std::enable_if<
    detail::variant::is_comparable<R, U>{}, bool
  >::type
  binary_relation(variant<A...> const& a) const noexcept
  {
    return I == type_id_ ?
      R<U>()(get<U>(), a.template get<U>()) :
      throw ::std::bad_typeid();
  }

  template <template <typename> class R, int I, typename U, typename ...V,
    typename ...A>
  typename ::std::enable_if<
    bool(sizeof...(V)) && !detail::variant::is_comparable<R, U>{}, bool
  >::type
  binary_relation(variant<A...> const& a) const noexcept
  {
    return I == type_id_ ?
      throw ::std::bad_typeid() :
      binary_relation<R, I + 1, V...>(a);
  }

  template <template <typename> class R, int I, typename U, typename ...V,
    typename ...A>
  typename ::std::enable_if<
    bool(sizeof...(V)) && detail::variant::is_comparable<R, U>{}, bool
  >::type
  binary_relation(variant<A...> const& a) const noexcept
  {
    return I == type_id_ ?
      R<U>()(get<U>(), a.template get<U>()) :
      binary_relation<R, I + 1, V...>(a);
  }

  template <::std::size_t I, typename OS, typename U>
  typename ::std::enable_if<
    !detail::variant::is_streamable<OS, U>{}, OS&
  >::type
  stream_value(OS& os) const
  {
    throw ::std::bad_typeid();
  }

  template <::std::size_t I, typename OS, typename U>
  typename ::std::enable_if<
    detail::variant::is_streamable<OS, U>{}, OS&
  >::type
  stream_value(OS& os) const
  {
    return I == type_id_ ? os << get<U>() : os;
  }

  template <::std::size_t I, typename OS, typename U, typename ...V>
  typename ::std::enable_if<
    !detail::variant::is_streamable<OS, U>{} && bool(sizeof...(V)), OS&
  >::type
  stream_value(OS& os) const
  {
    return I == type_id_ ?
      throw ::std::bad_typeid() :
      stream_value<I + 1, OS, V...>(os);
  }

  template <::std::size_t I, typename OS, typename U, typename ...V>
  typename ::std::enable_if<
    detail::variant::is_streamable<OS, U>{} && bool(sizeof...(V)), OS&
  >::type
  stream_value(OS& os) const
  {
    return I == type_id_ ?
      os << get<U>() :
      stream_value<I + 1, OS, V...>(os);
  }

  using deleter_type = void (*)(void*);
  using copier_type = void (*)(bool, deleter_type, void*, void const*);
  using mover_type = void (*)(bool, deleter_type, void*, void*);

  struct meta
  {
    deleter_type deleter;
    copier_type copier;
    mover_type mover;
  };

  template <typename U>
  static struct meta const* meta()
  {
    static const struct meta m{
      deleter_stub<U>,
      get_copier<U>(),
      get_mover<U>()
    };

    return &m;
  }

public:
  variant() = default;

  ~variant() { meta_->deleter(&store_); }

  variant(variant const& other) { *this = other; }

  variant(variant&& other) { *this = ::std::move(other); }

  bool operator==(variant const& v) const noexcept
  {
    return (v.type_id_ == type_id_) && *this ?
      binary_relation<::std::equal_to, 0, T...>(v) :
      type_id_ == v.type_id_;
  }

  template <typename ...U>
  bool operator==(variant<U...> const& v) const noexcept
  {
    auto const converted_type_id(v.template convert_type_id<0, T...>());

    return (converted_type_id == type_id_) && *this ?
      binary_relation<::std::equal_to, 0, T...>(v) :
      converted_type_id == type_id_;
  }

  bool operator<(variant const& v) const noexcept
  {
    return (v.type_id_ == type_id_) && *this ?
      binary_relation<::std::less, 0, T...>(v) :
      type_id_ < v.type_id_;
  }

  template <typename ...U>
  bool operator<(variant<U...> const& v) const noexcept
  {
    auto const converted_type_id(v.template convert_type_id<0, T...>());

    return (converted_type_id == type_id_) && *this?
      binary_relation<::std::less, 0, T...>(v) :
      type_id_ < converted_type_id;
  }

  bool operator<=(variant const& v) const noexcept
  {
    return (v.type_id_ == type_id_) && *this ?
      binary_relation<::std::less_equal, 0, T...>(v) :
      type_id_ <= v.type_id_;
  }

  template <typename ...U>
  bool operator<=(variant<U...> const& v) const noexcept
  {
    auto const converted_type_id(v.template convert_type_id<0, T...>());

    return (converted_type_id == type_id_) && *this ?
      binary_relation<::std::less_equal, 0, T...>(v) :
      type_id_ <= converted_type_id;
  }

  bool operator>(variant const& v) const noexcept
  {
    return (v.type_id_ == type_id_) && *this ?
      binary_relation<::std::greater, 0, T...>(v) :
      type_id_ > v.type_id_;
  }

  template <typename ...U>
  bool operator>(variant<U...> const& v) const noexcept
  {
    auto const converted_type_id(v.template convert_type_id<0, T...>());

    return converted_type_id == type_id_ ?
      binary_relation<::std::greater, 0, T...>(v) :
      type_id_ > converted_type_id;
  }

  bool operator>=(variant const& v) const noexcept
  {
    return v.type_id_ == type_id_ ?
      binary_relation<::std::greater_equal, 0, T...>(v) :
      type_id_ >= v.type_id_;
  }

  template <typename ...U>
  bool operator>=(variant<U...> const& v) const noexcept
  {
    auto const converted_type_id(v.template convert_type_id<0, T...>());

    return converted_type_id == type_id_ ?
      binary_relation<::std::greater_equal, 0, T...>(v) :
      type_id_ >= converted_type_id;
  }

  variant& operator=(variant const& rhs)
  {
    if (!rhs)
    {
      clear();
    }
    else if (rhs.meta_->copier)
    {
      rhs.meta_->copier(type_id_ == rhs.type_id_,
        meta_->deleter, store_, rhs.store_);

      meta_ = rhs.meta_;

      type_id_ = rhs.type_id_;
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
      auto const converted_type_id(
        rhs.template convert_type_id<0, T...>()
      );

      if (-1 == converted_type_id)
      {
        throw ::std::bad_typeid();
      }
      else
      {
        rhs.meta_->copier(type_id_ == converted_type_id,
          meta_->deleter, store_, rhs.store_);

        meta_ = rhs.meta_;

        type_id_ = converted_type_id;
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
      rhs.meta_->mover(type_id_ == rhs.type_id_,
        meta_->deleter, store_, rhs.store_);

      meta_ = rhs.meta_;

      type_id_ = rhs.type_id_;
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
      auto const converted_type_id(rhs.template convert_type_id<0, T...>());

      if (-1 == converted_type_id)
      {
        throw ::std::bad_typeid();
      }
      else
      {
        rhs.meta_.mover(type_id_ == converted_type_id,
          meta_->deleter, store_, rhs.store_);

        meta_ = rhs.meta_;

        type_id_ = converted_type_id;
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
    typename = typename ::std::enable_if<
      detail::variant::any_of<
        ::std::is_same<typename ::std::decay<U>::type, T>...>{} &&
        !::std::is_same<typename ::std::decay<U>::type,
        variant
      >{}
    >::type
  >
  variant(U&& f)
  {
    *this = ::std::forward<U>(f);
  }

  template <typename U>
  typename ::std::enable_if<
    detail::variant::any_of<
      ::std::is_same<typename ::std::decay<U>::type, T>...
    >{} &&
    ::std::is_copy_assignable<typename ::std::decay<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& u)
  {
    using user_type = typename ::std::decay<U>::type;

    if (detail::variant::index_of<user_type, T...>{} == type_id_)
    {
      *reinterpret_cast<user_type*>(store_) = u;
    }
    else
    {
      clear();

      new (static_cast<void*>(store_)) user_type(::std::forward<U>(u));

      meta_ = meta<user_type>();

      type_id_ = detail::variant::index_of<user_type, T...>{};
    }

    return *this;
  }

  template <typename U>
  typename ::std::enable_if<
    detail::variant::any_of<
      ::std::is_same<typename ::std::decay<U>::type, T>...
    >{} &&
    ::std::is_rvalue_reference<U&&>{} &&
    !::std::is_copy_assignable<typename ::std::decay<U>::type>{} &&
    ::std::is_move_assignable<typename ::std::decay<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& u)
  {
    using user_type = typename ::std::decay<U>::type;

    if (detail::variant::index_of<user_type, T...>{} == type_id_)
    {
      *reinterpret_cast<user_type*>(store_) = ::std::move(u);
    }
    else
    {
      clear();

      new (static_cast<void*>(store_)) user_type(::std::forward<U>(u));

      meta_ = meta<user_type>();

      type_id_ = detail::variant::index_of<user_type, T...>{};
    }

    return *this;
  }

  template <typename U>
  typename ::std::enable_if<
    detail::variant::any_of<
      ::std::is_same<typename ::std::decay<U>::type, T>...
    >{} &&
    !::std::is_copy_assignable<typename ::std::decay<U>::type>{} &&
    !::std::is_move_assignable<typename ::std::decay<U>::type>{} &&
    !::std::is_same<typename ::std::decay<U>::type, variant>{},
    variant&
  >::type
  operator=(U&& u)
  {
    using user_type = typename ::std::decay<U>::type;

    clear();

    new (static_cast<void*>(store_)) user_type(::std::forward<U>(u));

    meta_ = meta<user_type>();

    type_id_ = detail::variant::index_of<user_type, T...>{};

    return *this;
  }

  explicit operator bool() const noexcept { return -1 != type_id_; }

  template <typename U>
  variant& assign(U&& u)
  {
    return operator=(::std::forward<U>(u));
  }

  template <typename U>
  bool contains() const noexcept
  {
    return detail::variant::index_of<U, T...>{} == type_id_;
  }

  void clear()
  { 
    meta_->deleter(store_);

    meta_ = meta<void>();

    type_id_ = -1;
  }

  bool empty() const noexcept { return !*this; }

  void swap(variant& other)
  {
    if (-1 == other.type_id_)
    {
      if (meta_->mover)
      {
        other = ::std::move(*this);
        clear();
      }
      else if (meta_->copier)
      {
        other = *this;
        clear();
      }
      // else do nothing
    }
    else if (-1 == type_id_)
    {
      if (other.meta_->mover)
      {
        *this = ::std::move(other);
        other.clear();
      }
      else if (other.meta_->copier)
      {
        *this = other;
        other.clear();
      }
      // else do nothing
    }
    else if (meta_->mover && other.meta_->mover)
    {
      variant tmp(::std::move(other));

      other = ::std::move(*this);
      *this = ::std::move(tmp);
    }
    else if (meta_->mover && other.meta_->copier)
    {
      variant tmp(other);

      other = ::std::move(*this);
      *this = tmp;
    }
    else if (meta_->copier && other.meta_->mover)
    {
      variant tmp(::std::move(other));

      other = *this;
      *this = ::std::move(tmp);
    }
    else if (meta_->copier && other.meta_->copier)
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
    (-1 != detail::variant::index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  cget() const
  {
    return get<U>();
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 == detail::variant::index_of<U, T...>{}) &&
    (-1 != detail::variant::compatible_index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  cget() const
  {
    return get<U>();
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::variant::index_of<U, T...>{}) &&
    !(::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U const&
  >::type
  cget() const
  {
    return get<U>();
  }

  template <typename U>
  typename ::std::enable_if<
    (-1 != detail::variant::index_of<U, T...>{}),
    U&
  >::type
  get()
  {
    if (detail::variant::index_of<U, T...>{} == type_id_)
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
    (-1 != detail::variant::index_of<U, T...>{}),
    U const&
  >::type
  get() const
  {
    if (detail::variant::index_of<U, T...>{} == type_id_) 
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
    (-1 == detail::variant::index_of<U, T...>{}) &&
    (-1 != detail::variant::compatible_index_of<U, T...>{}) &&
    (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
    U
  >::type
  get() const
  {
    static_assert(::std::is_same<
      typename detail::variant::type_at<
        detail::variant::compatible_index_of<U, T...>{}, T...>::type,
      typename detail::variant::compatible_type<U, T...>::type>{},
      "internal error");
    if (detail::variant::compatible_index_of<U, T...>{} == type_id_)
    {
      return U(
        *reinterpret_cast<
          typename detail::variant::compatible_type<U, T...>::type const*
        >(store_)
      );
    }
    else
    {
      throw ::std::bad_typeid();
    }
  }

  template <typename U>
  static constexpr int type_id() noexcept
  {
    return detail::variant::index_of<U, T...>{};
  }

  int type_id() const noexcept { return type_id_; }

private:
  template <typename charT, typename traits>
  friend ::std::basic_ostream<charT, traits>& operator<<(
    ::std::basic_ostream<charT, traits>& os, variant const& v)
  {
    return -1 == v.type_id_ ?
      os << "<empty variant>" :
      v.stream_value<0, decltype(os), T...>(os);
  }

  template <class U>
  typename ::std::enable_if<
    ::std::is_copy_constructible<U>{}, copier_type
  >::type
  static get_copier() noexcept
  {
    return copier_stub<U>;
  }

  template <class U>
  typename ::std::enable_if<
    !::std::is_copy_constructible<U>{}, copier_type
  >::type
  static get_copier() noexcept
  {
    return nullptr;
  }

  template <class U>
  typename ::std::enable_if<
    ::std::is_move_constructible<U>{}, mover_type
  >::type
  static get_mover() noexcept
  {
    return mover_stub<U>;
  }

  template <class U>
  typename ::std::enable_if<
    !::std::is_move_constructible<U>{}, mover_type
  >::type
  static get_mover() noexcept
  {
    return nullptr;
  }

  template <class U>
  static typename ::std::enable_if<
    !::std::is_same<U, void>{}
  >::type
  deleter_stub(void* const store)
  {
    reinterpret_cast<U*>(store)->~U();
  }

  template <class U>
  static typename ::std::enable_if<
    ::std::is_same<U, void>{}
  >::type
  deleter_stub(void* const store) noexcept
  {
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
  int type_id_{-1};

  struct meta const* meta_{meta<void>()};

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
      auto const seed(v.template get_hash<0, T...>());

      return hash<decltype(v.type_id())>()(v.type_id()) +
        0x9e3779b9 + (seed << 6) + (seed >> 2);
    }
  };
}

#endif // GENERIC_VARIANT_HPP
