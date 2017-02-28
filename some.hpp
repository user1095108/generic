#ifndef GNR_SOME_HPP
# define GNR_SOME_HPP
# pragma once

#include <cassert>

#include <ostream>

#include <type_traits>

#include <typeinfo>

#include <utility>

namespace gnr
{

namespace detail
{

namespace some
{

template <typename T>
using remove_cvr_t = std::remove_cv_t<std::remove_reference_t<T>>;

template <bool B>
using bool_constant = std::integral_constant<bool, B>;

template <class A, class ...B>
struct any_of : bool_constant<A::value || any_of<B...>::value>
{
};

template <class A>
struct any_of<A> : bool_constant<A::value>
{
};

template <typename T, typename = std::size_t>
struct is_vector : std::false_type
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
> : std::true_type
{
};

template <typename T, typename = std::size_t>
struct is_list : std::false_type
{
};

template <typename T>
struct is_list<T,
  decltype(
    sizeof((void(T::*)(typename T::const_reference))(&T::push_front)) |
    sizeof((void(T::*)(typename T::value_type&&))(&T::push_front)) |
    sizeof(&T::pop_front)
  )
> : std::true_type
{
};

template <typename T, typename = std::size_t>
struct is_map : std::false_type
{
};

template <typename T>
struct is_map<T,
  decltype(
    sizeof(typename T::key_type) |
    sizeof(typename T::mapped_type) |
    sizeof(typename T::value_type)
  )
> : std::true_type
{
};

template <typename T, typename = void>
struct is_copy_assignable : std::is_copy_assignable<T>
{
};

template <typename T>
struct is_copy_assignable<T,
  typename std::enable_if<is_vector<T>{}>::type
> : is_copy_assignable<typename T::value_type>
{
};

template <typename T>
struct is_copy_assignable<T,
  std::enable_if_t<is_list<T>{}>
> : is_copy_assignable<typename T::value_type>
{
};

template <typename T>
struct is_copy_assignable<T,
  std::enable_if_t<is_map<T>{}>
> : is_copy_assignable<typename T::mapped_type>
{
};

template <typename T, typename = void>
struct is_copy_constructible : std::is_copy_constructible<T>
{
};

template <typename T>
struct is_copy_constructible<T,
  typename std::enable_if<is_vector<T>{}>::type
> : is_copy_constructible<typename T::value_type>
{
};

template <typename T>
struct is_copy_constructible<T,
  typename std::enable_if<is_list<T>{}>::type
> : is_copy_constructible<typename T::value_type>
{
};

template <typename T>
struct is_copy_constructible<T,
  typename std::enable_if<is_map<T>{}>::type
> : is_copy_constructible<typename T::mapped_type>
{
};

template <typename T, typename = void>
struct is_move_assignable : std::is_move_assignable<T>
{
};

template <typename T>
struct is_move_assignable<T, std::enable_if_t<is_vector<T>{}> > :
  is_move_assignable<typename T::value_type>
{
};

template <typename T>
struct is_move_assignable<T, std::enable_if_t<is_list<T>{}> > :
  is_move_assignable<typename T::value_type>
{
};


template <typename T>
struct is_move_assignable<T, std::enable_if_t<is_map<T>{}> > :
  is_move_assignable<typename T::mapped_type>
{
};

template <typename T, typename = void>
struct is_move_constructible : std::is_move_constructible<T>
{
};

template <typename T>
struct is_move_constructible<T, std::enable_if_t<is_vector<T>{}> > :
  is_move_constructible<typename T::value_type>
{
};

template <typename T>
struct is_move_constructible<T, std::enable_if_t<is_list<T>{}> > :
  is_move_constructible<typename T::value_type>
{
};

template <typename T>
struct is_move_constructible<T, std::enable_if<is_map<T>{}> > :
  is_move_constructible<typename T::mapped_type>
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
typename std::enable_if_t<!std::is_void<U>{}>
deleter_stub(void* const store)
{
  reinterpret_cast<U*>(store)->~U();
}

template <class U>
typename std::enable_if_t<std::is_void<U>{}>
deleter_stub(void* const)
{
}

template <typename U>
typename std::enable_if_t<
  is_copy_constructible<U>{} &&
  is_copy_assignable<U>{}
>
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
typename std::enable_if_t<
  is_copy_constructible<U>{} &&
  !is_copy_assignable<U>{}
>
copier_stub(bool const, deleter_type const deleter,
  void* const dst_store, void const* const src_store)
{
  deleter(dst_store);

  new (dst_store) U(*reinterpret_cast<U const*>(src_store));
}

template <typename U>
typename std::enable_if_t<
  is_move_constructible<U>{} &&
  is_move_assignable<U>{}
>
mover_stub(bool const same_type, deleter_type const deleter,
  void* const dst_store, void* const src_store)
{
  if (same_type)
  {
    *reinterpret_cast<U*>(dst_store) =
      std::move(*reinterpret_cast<U*>(src_store));
  }
  else
  {
    deleter(dst_store);

    new (dst_store) U(std::move(*reinterpret_cast<U*>(src_store)));
  }
}

template <typename U>
typename std::enable_if_t<
  is_move_constructible<U>{} &&
  !is_move_assignable<U>{}
>
mover_stub(bool const, deleter_type const deleter,
  void* const dst_store, void* const src_store)
{
  deleter(dst_store);

  new (dst_store) U(std::move(*reinterpret_cast<U*>(src_store)));
}

template <class U>
constexpr inline std::enable_if_t<!is_copy_constructible<U>{}, copier_type>
get_copier() noexcept
{
  return nullptr;
}

template <class U>
constexpr inline std::enable_if_t<is_copy_constructible<U>{}, copier_type>
get_copier() noexcept
{
  return copier_stub<U>;
}

template <class U>
constexpr inline std::enable_if_t<!is_move_constructible<U>{}, mover_type>
get_mover() noexcept
{
  return nullptr;
}

template <class U>
constexpr inline std::enable_if_t<is_move_constructible<U>{}, mover_type>
get_mover() noexcept
{
  return mover_stub<U>;
}

using typeid_t = void(*)();

struct meta
{
  copier_type copier;
  mover_type mover;
  deleter_type deleter;

  std::size_t size;

  typeid_t type_id;
};

template <typename U>
inline std::enable_if_t<!std::is_void<U>{}, struct meta const*>
get_meta()
{
  static struct meta const m{
    get_copier<U>(),
    get_mover<U>(),
    deleter_stub<U>,
    sizeof(U),
    typeid_t(get_meta<U>)
  };

  return &m;
}

template <typename U>
inline std::enable_if_t<std::is_void<U>{}, struct meta const*>
get_meta()
{
  static struct meta const m{
    get_copier<U>(),
    get_mover<U>(),
    deleter_stub<U>,
    0,
    typeid_t(get_meta<U>)
  };

  return &m;
}

}

}

#ifdef __GNUC__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif // __GNUC__

template <std::size_t N>
class some
{
  template <std::size_t M>
  friend class some;

public:
  using typeid_t = detail::some::typeid_t;

  some() = default;

  ~some() { meta_->deleter(&store_); }

  some(some const& other) { *this = other; }

  some(some&& other) { *this = std::move(other); }

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
        throw std::bad_typeid();
#endif // NDEBUG
      }
    }
    // else do nothing

    return *this;
  }

  template <std::size_t M>
  some& operator=(some<M> const& rhs)
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
        throw std::bad_typeid();
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
        throw std::bad_typeid();
#endif // NDEBUG
      }
    }
    // else do nothing

    return *this;
  }

  template <std::size_t M>
  some& operator=(some<M>&& rhs)
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
        throw std::bad_typeid();
#endif // NDEBUG
      }
    }
    // else do nothing

    return *this;
  }

  template <
    typename U,
    typename = std::enable_if_t<
      !std::is_array<
        typename detail::some::remove_cvr_t<U>
      >{} &&
      !std::is_same<
        typename std::decay<U>::type, some
      >{}
    >
  >
  some(U&& f)
  {
    *this = std::forward<U>(f);
  }

  explicit operator bool() const noexcept
  {
    return detail::some::get_meta<void>() != meta_;
  }

  template <typename U,
    typename = std::enable_if_t<
      !std::is_same<
        typename std::decay<U>::type, some
      >{}
    >
  >
  some& operator=(U&& u)
  {
    assign(std::forward<U>(u));

    return *this;
  }

  // copy assignment possible
  template <typename U>
  std::enable_if_t<
    !std::is_array<detail::some::remove_cvr_t<U> >{} &&
    detail::some::is_copy_assignable<detail::some::remove_cvr_t<U> >{},
    some&
  >
  assign(U&& u)
  {
    using user_type = std::decay_t<U>;
    static_assert(sizeof(user_type) <= sizeof(store_), "");

    if (detail::some::get_meta<user_type>() == meta_)
    {
      *reinterpret_cast<user_type*>(&store_) = u;
    }
    else
    {
      clear();

      new (static_cast<void*>(&store_)) user_type(std::forward<U>(u));

      meta_ = detail::some::get_meta<user_type>();
    }

    return *this;
  }

  // move assignment possible
  template <typename U>
  std::enable_if_t<
    !std::is_array<detail::some::remove_cvr_t<U> >{} &&
    std::is_rvalue_reference<U&&>{} &&
    !detail::some::is_copy_assignable<detail::some::remove_cvr_t<U> >{} &&
    detail::some::is_move_assignable<detail::some::remove_cvr_t<U> >{},
    some&
  >
  assign(U&& u)
  {
    using user_type = std::decay_t<U>;
    static_assert(sizeof(user_type) <= sizeof(store_), "");

    if (detail::some::get_meta<user_type>() == meta_)
    {
      *reinterpret_cast<user_type*>(&store_) = std::move(u);
    }
    else
    {
      clear();

      new (static_cast<void*>(&store_)) user_type(std::forward<U>(u));

      meta_ = detail::some::get_meta<user_type>();
    }

    return *this;
  }

  // assignment not possible
  template <typename U>
  std::enable_if_t<
    !std::is_array<detail::some::remove_cvr_t<U> >{} &&
    !detail::some::is_copy_assignable<detail::some::remove_cvr_t<U> >{} &&
    !detail::some::is_move_assignable<detail::some::remove_cvr_t<U> >{},
    some&
  >
  assign(U&& u)
  {
    using user_type = std::decay_t<U>;
    static_assert(sizeof(user_type) <= sizeof(store_), "");

    clear();

    new (static_cast<void*>(&store_)) user_type(std::forward<U>(u));

    meta_ = detail::some::get_meta<user_type>();

    return *this;
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
        other = std::move(*this);
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
        *this = std::move(other);
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
      some tmp(std::move(other));

      other = std::move(*this);
      *this = std::move(tmp);
    }
    else if (meta_->mover && other.meta_->copier)
    {
      some tmp(other);

      other = std::move(*this);
      *this = tmp;
    }
    else if (meta_->copier && other.meta_->mover)
    {
      some tmp(std::move(other));

      other = *this;
      *this = std::move(tmp);
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
      throw std::bad_typeid();
#endif // NDEBUG
    }
  }

  template <typename U>
  static typeid_t type_id() noexcept
  {
    return typeid_t(detail::some::get_meta<U>());
  }

  typeid_t type_id() const noexcept { return meta_->type_id; }

private:
  template <typename U, std::size_t M> friend bool contains(some<M> const&) noexcept;

#ifdef NDEBUG
  template <typename U, std::size_t M> friend U& std::get(some<M>&) noexcept;
  template <typename U, std::size_t M> friend U const& std::get(some<M> const&) noexcept;
#else
  template <typename U, std::size_t M> friend U& std::get(some<M>&);
  template <typename U, std::size_t M> friend U const& std::get(some<M> const&);
#endif // NDEBUG

  struct detail::some::meta const* meta_{detail::some::get_meta<void>()};

  typename std::aligned_storage<N>::type store_;
};

template <typename U, std::size_t N>
inline bool contains(some<N> const& s) noexcept
{
  return detail::some::get_meta<U>() == s.meta_;
}

#ifdef __GNUC__
# pragma GCC diagnostic pop
#endif // __GNUC__

}

namespace std
{

template <typename U, std::size_t N>
inline U& get(gnr::some<N>& s)
#ifdef NDEBUG
noexcept
#endif // NDEBUG
{
  using nonref = gnr::detail::some::remove_cvr_t<U>;

#ifndef NDEBUG
  if (gnr::contains<nonref>(s))
  {
    return *reinterpret_cast<nonref*>(&s.store_);
  }
  else
  {
    throw std::bad_cast();
  }
#else
  return *reinterpret_cast<nonref*>(&s.store_);
#endif // NDEBUG
}

template <typename U, std::size_t N>
inline U const& get(gnr::some<N> const& s)
#ifdef NDEBUG
noexcept
#endif // NDEBUG
{
  using nonref = gnr::detail::some::remove_cvr_t<U>;

#ifndef NDEBUG
  if (gnr::contains<nonref>(s))
  {
    return *reinterpret_cast<nonref const*>(&s.store_);
  }
  else
  {
    throw std::bad_cast();
  }
#else
  return *reinterpret_cast<nonref const*>(&s.store_);
#endif // NDEBUG
}

}

#endif // GNR_SOME_HPP
