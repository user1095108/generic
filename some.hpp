#ifndef GENERIC_SOME_HPP
# define GENERIC_SOME_HPP
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

namespace some
{

template <typename T>
using remove_cvr = ::std::remove_cv<
  typename ::std::remove_reference<T>::type
>;

template <typename A, typename ...B>
struct max_type_size : ::std::conditional<
  (sizeof(A) > max_type_size<B...>{}),
  ::std::integral_constant<::std::size_t, sizeof(A)>,
  max_type_size<B...>
>::type
{
};

template <typename A, typename B>
struct max_type_size<A, B> : ::std::conditional<
  (sizeof(A) > sizeof(B)),
  ::std::integral_constant<::std::size_t, sizeof(A)>,
  ::std::integral_constant<::std::size_t, sizeof(B)>
>::type
{
};

template <typename A>
struct max_type_size<A> : ::std::integral_constant<
  ::std::size_t, sizeof(A)
>::type
{
};

template <bool B>
using bool_constant = ::std::integral_constant<bool, B>;

template <class A, class ...B>
struct any_of : bool_constant<A::value || any_of<B...>::value>
{
};

template <class A>
struct any_of<A> : bool_constant<A::value>
{
};

template <typename T, typename = ::std::size_t>
struct is_vector : ::std::false_type
{
};

template <typename T>
struct is_vector<T,
  decltype(
    sizeof((typename T::reference(T::*)())(&T::back)) |
    sizeof((typename T::reference(T::*)())(&T::front)) |
    sizeof((typename T::value_type const*(T::*)() const)(&T::data)) |
    sizeof((typename T::value_type*(T::*)())(&T::data)) |
    sizeof((void(T::*)(typename T::const_reference))(&T::push_back)) |
    sizeof((void(T::*)(typename T::value_type&&))(&T::push_back)) |
    sizeof(&T::shrink_to_fit)
  )
> : ::std::true_type
{
};

template <typename T, typename = ::std::size_t>
struct is_list : ::std::false_type
{
};

template <typename T>
struct is_list<T,
  decltype(
    sizeof((void(T::*)(typename T::const_reference))(&T::push_front)) |
    sizeof((void(T::*)(typename T::value_type&&))(&T::push_front)) |
    sizeof(&T::pop_front)
  )
> : ::std::true_type
{
};

template <typename T, typename = ::std::size_t>
struct is_map : ::std::false_type
{
};

template <typename T>
struct is_map<T,
  decltype(
    sizeof(typename T::key_type) |
    sizeof(typename T::mapped_type) |
    sizeof(typename T::value_type)
  )
> : ::std::true_type
{
};

template <typename T, typename = void>
struct is_copy_assignable : ::std::is_copy_assignable<T>
{
};

template <typename T>
struct is_copy_assignable<T,
  typename ::std::enable_if<is_vector<T>{}>::type
> : is_copy_assignable<typename T::value_type>
{
};

template <typename T>
struct is_copy_assignable<T,
  typename ::std::enable_if<is_list<T>{}>::type
> : is_copy_assignable<typename T::value_type>
{
};

template <typename T>
struct is_copy_assignable<T,
  typename ::std::enable_if<is_map<T>{}>::type
> : is_copy_assignable<typename T::mapped_type>
{
};

template <typename T, typename = void>
struct is_copy_constructible : ::std::is_copy_constructible<T>
{
};

template <typename T>
struct is_copy_constructible<T,
  typename ::std::enable_if<is_vector<T>{}>::type
> : is_copy_constructible<typename T::value_type>
{
};

template <typename T>
struct is_copy_constructible<T,
  typename ::std::enable_if<is_list<T>{}>::type
> : is_copy_constructible<typename T::value_type>
{
};

template <typename T>
struct is_copy_constructible<T,
  typename ::std::enable_if<is_map<T>{}>::type
> : is_copy_constructible<typename T::mapped_type>
{
};

template <typename T, typename = void>
struct is_move_assignable : detail::some::is_move_assignable<T>
{
};

template <typename T>
struct is_move_assignable<T,
  typename ::std::enable_if<is_vector<T>{}>::type
> : is_move_assignable<typename T::value_type>
{
};

template <typename T>
struct is_move_assignable<T,
  typename ::std::enable_if<is_list<T>{}>::type
> : is_move_assignable<typename T::value_type>
{
};


template <typename T>
struct is_move_assignable<T,
  typename ::std::enable_if<is_map<T>{}>::type
> : is_move_assignable<typename T::mapped_type>
{
};

template <typename T, typename = void>
struct is_move_constructible : ::std::is_move_constructible<T>
{
};

template <typename T>
struct is_move_constructible<T,
  typename ::std::enable_if<is_vector<T>{}>::type
> : is_move_constructible<typename T::value_type>
{
};

template <typename T>
struct is_move_constructible<T,
  typename ::std::enable_if<is_list<T>{}>::type
> : is_move_constructible<typename T::value_type>
{
};

template <typename T>
struct is_move_constructible<T,
  typename ::std::enable_if<is_map<T>{}>::type
> : is_move_constructible<typename T::mapped_type>
{
};

template <class A>
struct is_move_or_copy_constructible :
  bool_constant<is_copy_constructible<A>{} || is_move_constructible<A>{}>
{
};

using deleter_type = void (*)(void*);
using copier_type = void (*)(bool, deleter_type, void*, void const*);
using mover_type = void (*)(bool, deleter_type, void*, void*);

template <class U>
typename ::std::enable_if<
  !::std::is_void<U>{}
>::type
deleter_stub(void* const store)
{
  reinterpret_cast<U*>(store)->~U();
}

template <class U>
typename ::std::enable_if<
  ::std::is_void<U>{}
>::type
deleter_stub(void* const)
{
}

template <typename U>
typename ::std::enable_if<
  is_copy_constructible<U>{} &&
  is_copy_assignable<U>{}
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
typename ::std::enable_if<
  is_copy_constructible<U>{} &&
  !is_copy_assignable<U>{}
>::type
copier_stub(bool const, deleter_type const deleter,
  void* const dst_store, void const* const src_store)
{
  deleter(dst_store);

  new (dst_store) U(*reinterpret_cast<U const*>(src_store));
}

template <typename U>
typename ::std::enable_if<
  is_move_constructible<U>{} &&
  is_move_assignable<U>{}
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
typename ::std::enable_if<
  is_move_constructible<U>{} &&
  !is_move_assignable<U>{}
>::type
mover_stub(bool const, deleter_type const deleter,
  void* const dst_store, void* const src_store)
{
  deleter(dst_store);

  new (dst_store) U(::std::move(*reinterpret_cast<U*>(src_store)));
}

template <class U>
inline typename ::std::enable_if<
  !is_copy_constructible<U>{}, copier_type
>::type
get_copier() noexcept
{
  return nullptr;
}

template <class U>
inline typename ::std::enable_if<
  is_copy_constructible<U>{}, copier_type
>::type
get_copier() noexcept
{
  return copier_stub<U>;
}

template <class U>
inline typename ::std::enable_if<
  !is_move_constructible<U>{}, mover_type
>::type
get_mover() noexcept
{
  return nullptr;
}

template <class U>
inline typename ::std::enable_if<
  is_move_constructible<U>{}, mover_type
>::type
get_mover() noexcept
{
  return mover_stub<U>;
}

struct meta
{
  deleter_type deleter;
  copier_type copier;
  mover_type mover;

  ::std::size_t size;
};

template <typename U>
inline typename ::std::enable_if<
  !::std::is_void<U>{},
  struct meta const*
>::type
get_meta()
{
  static struct meta const m{
    deleter_stub<U>,
    get_copier<U>(),
    get_mover<U>(),
    sizeof(U)
  };

  return &m;
}

template <typename U>
inline typename ::std::enable_if<
  ::std::is_void<U>{},
  struct meta const*
>::type
get_meta()
{
  static struct meta const m{
    deleter_stub<U>,
    get_copier<U>(),
    get_mover<U>(),
    0
  };

  return &m;
}

}

}

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif // __GNUC__

template <typename ...T>
class some
{
  static_assert(!detail::some::any_of< ::std::is_reference<T>...>{},
    "reference types are unsupported");
  static_assert(!detail::some::any_of< ::std::is_void<T>...>{},
    "void type is unsupported");

  template <typename ...U>
  friend class some;

public:
  using typeid_t = ::std::uintptr_t;

  some() = default;

  ~some() { meta_->deleter(&store_); }

  some(some const& other) { *this = other; }

  some(some&& other) { *this = ::std::move(other); }

  some& operator=(some const& rhs)
  {
    if (this != &rhs)
    {
      if (!rhs)
      {
        clear();
      }
      else if (rhs.meta_->copier)
      {
        rhs.meta_->copier(meta_ == rhs.meta_,
          meta_->deleter, &store_, &rhs.store_);

        meta_ = rhs.meta_;
      }
      else
      {
#ifndef NDEBUG
        throw ::std::bad_typeid();
#endif // NDEBUG
      }
    }
    // else do nothing

    return *this;
  }

  template <typename ...U>
  some& operator=(some<U...> const& rhs)
  {
    if (this != &rhs)
    {
      if (!rhs)
      {
        clear();
      }
      else if ((rhs.meta_->copier_) &&
        (rhs.meta_->size <= sizeof(store_)))
      {
        rhs.meta_->copier(meta_ == rhs.meta_,
          meta_->deleter, &store_, &rhs.store_);

        meta_ = rhs.meta_;
      }
      else
      {
#ifndef NDEBUG
        throw ::std::bad_typeid();
#endif // NDEBUG
      }
    }
    // else do nothing

    return *this;
  }

  some& operator=(some&& rhs)
  {
    if (this != &rhs)
    {
      if (!rhs)
      {
        clear();
      }
      else if (rhs.meta_->mover)
      {
        rhs.meta_->mover(meta_ == rhs.meta_,
          meta_->deleter, &store_, &rhs.store_);

        meta_ = rhs.meta_;
      }
      else
      {
#ifndef NDEBUG
        throw ::std::bad_typeid();
#endif // NDEBUG
      }
    }
    // else do nothing

    return *this;
  }

  template <typename ...U>
  some& operator=(some<U...>&& rhs)
  {
    if (this != &rhs)
    {
      if (!rhs)
      {
        clear();
      }
      else if ((rhs.mover_) &&
        (rhs.meta_->size <= sizeof(store_)))
      {
        rhs.meta_.mover(meta_ == rhs.meta_,
          meta_->deleter, &store_, &rhs.store_);

        meta_ = rhs.meta_;
      }
      else
      {
#ifndef NDEBUG
        throw ::std::bad_typeid();
#endif // NDEBUG
      }
    }
    // else do nothing

    return *this;
  }

  template <
    typename U,
    typename = typename ::std::enable_if<
      !::std::is_array<
        typename detail::some::remove_cvr<U>::type
      >{} &&
      !::std::is_same<
        typename ::std::decay<U>::type, some
      >{}
    >::type
  >
  some(U&& f)
  {
    *this = ::std::forward<U>(f);
  }

  // copy assignment possible
  template <typename U>
  typename ::std::enable_if<
    !::std::is_array<
      typename detail::some::remove_cvr<U>::type
    >{} &&
    detail::some::is_copy_assignable<
      typename detail::some::remove_cvr<U>::type
    >{} &&
    !::std::is_same<
      typename ::std::decay<U>::type, some
    >{},
    some&
  >::type
  operator=(U&& u)
  {
    static_assert(sizeof(U) <= sizeof(store_), "");
    using user_type = typename detail::some::remove_cvr<U>::type;

    if (detail::some::get_meta<user_type>() == meta_)
    {
      *reinterpret_cast<user_type*>(&store_) = u;
    }
    else
    {
      clear();

      new (static_cast<void*>(&store_)) user_type(::std::forward<U>(u));

      meta_ = detail::some::get_meta<user_type>();
    }

    return *this;
  }

  // move assignment possible
  template <typename U>
  typename ::std::enable_if<
    !::std::is_array<
      typename detail::some::remove_cvr<U>::type
    >{} &&
    ::std::is_rvalue_reference<U&&>{} &&
    !detail::some::is_copy_assignable<
      typename detail::some::remove_cvr<U>::type
    >{} &&
    detail::some::is_move_assignable<
      typename detail::some::remove_cvr<U>::type
    >{} &&
    !::std::is_same<
      typename ::std::decay<U>::type, some
    >{},
    some&
  >::type
  operator=(U&& u)
  {
    static_assert(sizeof(U) <= sizeof(store_), "");
    using user_type = typename detail::some::remove_cvr<U>::type;

    if (detail::some::get_meta<user_type>() == meta_)
    {
      *reinterpret_cast<user_type*>(store_) = ::std::move(u);
    }
    else
    {
      clear();

      new (static_cast<void*>(&store_)) user_type(::std::forward<U>(u));

      meta_ = detail::some::get_meta<user_type>();
    }

    return *this;
  }

  // assignment not possible
  template <typename U>
  typename ::std::enable_if<
    !::std::is_array<
      typename detail::some::remove_cvr<U>::type
    >{} &&
    !detail::some::is_copy_assignable<
      typename detail::some::remove_cvr<U>::type
    >{} &&
    !detail::some::is_move_assignable<
      typename detail::some::remove_cvr<U>::type
    >{} &&
    !::std::is_same<
      typename ::std::decay<U>::type, some
    >{},
    some&
  >::type
  operator=(U&& u)
  {
    static_assert(sizeof(U) <= sizeof(store_), "");
    using user_type = typename detail::some::remove_cvr<U>::type;

    clear();

    new (static_cast<void*>(&store_)) user_type(::std::forward<U>(u));

    meta_ = detail::some::get_meta<user_type>();

    return *this;
  }

  explicit operator bool() const noexcept
  {
    return detail::some::get_meta<void>() != meta_;
  }

  template <typename U>
  some& assign(U&& u)
  {
    return operator=(::std::forward<U>(u));
  }

  template <typename U>
  bool contains() const noexcept
  {
    return detail::some::get_meta<U>() == meta_;
  }

  void clear()
  { 
    meta_->deleter(&store_);

    meta_ = detail::some::get_meta<void>();
  }

  bool empty() const noexcept { return !*this; }

  void swap(some& other)
  {
    if (detail::some::get_meta<void>() == other.meta_)
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
    else if (detail::some::get_meta<void>() == meta_)
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
      some tmp(::std::move(other));

      other = ::std::move(*this);
      *this = ::std::move(tmp);
    }
    else if (meta_->mover && other.meta_->copier)
    {
      some tmp(other);

      other = ::std::move(*this);
      *this = tmp;
    }
    else if (meta_->copier && other.meta_->mover)
    {
      some tmp(::std::move(other));

      other = *this;
      *this = ::std::move(tmp);
    }
    else if (meta_->copier && other.meta_->copier)
    {
      some tmp(other);

      other = *this;
      *this = tmp;
    }
    else
    {
#ifndef NDEBUG
      throw ::std::bad_typeid();
#endif // NDEBUG
    }
  }

  template <typename U>
  static typeid_t type_id() noexcept
  {
    return typeid_t(detail::some::get_meta<U>());
  }

  typeid_t type_id() const noexcept { return typeid_t(meta_); }

private:
private:
#ifdef NDEBUG
  template <typename U, typename ...V> friend U& get(some<V...>&) noexcept;
  template <typename U, typename ...V> friend U const& get(some<V...> const&) noexcept;
#else
  template <typename U, typename ...V> friend U& get(some<V...>&);
  template <typename U, typename ...V> friend U const& get(some<V...> const&);
#endif // NDEBUG

  struct detail::some::meta const* meta_{detail::some::get_meta<void>()};

  typename ::std::aligned_storage<
    detail::some::max_type_size<T...>{}
  >::type store_;
};

template <typename U, typename ...T>
inline U& get(some<T...>& s)
#ifdef NDEBUG
noexcept
#endif // NDEBUG
{
  using nonref = typename detail::some::remove_cvr<U>::type;

#ifndef NDEBUG
  if (s.template contains<nonref>())
  {
    return *reinterpret_cast<nonref*>(&s.store_);
  }
  else
  {
    throw ::std::bad_cast();
  }
#else
  return *reinterpret_cast<nonref*>(&s.store_);
#endif // NDEBUG
}

template <typename U, typename ...T>
inline U const& get(some<T...> const& s)
#ifdef NDEBUG
noexcept
#endif // NDEBUG
{
  using nonref = typename detail::some::remove_cvr<U>::type;

#ifndef NDEBUG
  if (s.template contains<nonref>())
  {
    return *reinterpret_cast<nonref const*>(&s.store_);
  }
  else
  {
    throw ::std::bad_cast();
  }
#else
  return *reinterpret_cast<nonref const*>(&s.store_);
#endif // NDEBUG
}

template <typename U, typename ...T>
inline typename ::std::enable_if<
  (::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
  U
>::type
cget(some<T...> const& s) noexcept(noexcept(get<U>(s)))
{
  return get<U>(s);
}

template <typename U, typename ...T>
inline typename ::std::enable_if<
  !(::std::is_enum<U>{} || ::std::is_fundamental<U>{}),
  U const&
>::type
cget(some<T...> const& s) noexcept(noexcept(get<U>(s)))
{
  return get<U>(s);
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif // __GNUC__

}

#endif // GENERIC_SOME_HPP
