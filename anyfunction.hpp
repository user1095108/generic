#ifndef GENERIC_ANYFUNCTION_HPP
# define GENERIC_ANYFUNCTION_HPP
# pragma once

#include <cstdint>

#include "many.hpp"

#include "some.hpp"

namespace generic
{

namespace
{

template <typename>
struct signature
{
};

template <typename F>
struct remove_cv_seq;

//
template <typename R, typename ...A>
struct remove_cv_seq<R(A...)>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) volatile>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const volatile>
{
  using type = R(A...);
};

//
template <typename R, typename ...A>
struct remove_cv_seq<R(A...) &>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const &>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) volatile &>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const volatile &>
{
  using type = R(A...);
};

//
template <typename R, typename ...A>
struct remove_cv_seq<R(A...) &&>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const &&>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) volatile &&>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const volatile &&>
{
  using type = R(A...);
};

template <typename F>
constexpr inline auto extract_signature(F* const) noexcept
{
  return signature<typename remove_cv_seq<F>::type>();
}

template <typename C, typename F>
constexpr inline auto extract_signature(F C::* const) noexcept
{
  return signature<typename remove_cv_seq<F>::type>();
}

template <typename F>
constexpr inline auto extract_signature(F const&) noexcept ->
  decltype(&F::operator(), extract_signature(&F::operator()))
{
  return extract_signature(&F::operator());
}

}

template <typename Any = ::generic::some<4 * sizeof(void*)>>
class any_function
{
  using typeid_t = ::std::uintptr_t;

  #if defined(__GNUC__)
  template <typename T>
  static typeid_t type_id() noexcept
  {
    return typeid_t(type_id<T>);
  }
  #else
  template <typename T>
  static typeid_t type_id() noexcept
  {
    static char const type_id{};

    return typeid_t(&type_id);
  }
  #endif // __GNUC__

  void (*f_)(Any const&, void const*, void*);

  Any any_;

#ifndef NDEBUG
  typeid_t type_id_;
#endif // NDEBUG

  template <typename F, typename R, typename ...A, ::std::size_t ...I>
  static ::std::enable_if_t<!::std::is_void<R>{}>
  do_invoke(F const& f,
    ::generic::many<A...> const& t,
    void* const r,
    ::std::index_sequence<I...> const) noexcept(
      noexcept(f(::std::get<I>(t)...))
    )
  {
    if (r)
    {
      ::new (r) R(f(::std::get<I>(t)...));
    }
    else
    {
      f(::std::get<I>(t)...);
    }
  }

  template <typename F, typename R, typename ...A, ::std::size_t ...I>
  static ::std::enable_if_t<::std::is_void<R>{}>
  do_invoke(F const& f,
    ::generic::many<A...> const& t,
    void* const,
    ::std::index_sequence<I...> const) noexcept(
      noexcept(f(::std::get<I>(t)...))
    )
  {
    f(::std::get<I>(t)...);
  }

  template <typename F, typename R, typename ...A>
  static void invoker(Any const& any,
    void const* const v,
    void* const r) noexcept(
    noexcept(do_invoke<F, R, A...>(::generic::get<F>(any),
      *static_cast<::generic::many<A...> const*>(v),
      r,
      ::std::make_index_sequence<sizeof...(A)>())
    )
  )
  {
    do_invoke<F, R, A...>(::generic::get<F>(any),
      *static_cast<::generic::many<A...> const*>(v),
      r,
      ::std::make_index_sequence<sizeof...(A)>()
    );
  }

  template <typename F, typename R, typename ...A>
  void assign(signature<R(A...)>) noexcept
  {
    f_ = invoker<F, R, A...>;

#ifndef NDEBUG
    type_id_ = type_id<::generic::many<A...> >();
#endif // NDEBUG
  }

public:
  template <typename T>
  struct arg_type
  {
    using type = ::std::decay_t<T>;
  };

  template <typename T>
  struct arg_type<::std::reference_wrapper<T> >
  {
    using type = T&;
  };

  template <typename T>
  using arg_type_t = typename arg_type<T>::type;

  any_function() = default;

  template <typename F,
    typename = ::std::enable_if_t<
      !::std::is_same<::std::decay_t<F>, any_function>{}
    >
  >
  any_function(F&& f) :
    any_(::std::forward<F>(f))
  {
    assign<F>(extract_signature(::std::forward<F>(f)));
  }

  any_function(any_function const&) = default;

  any_function(any_function&&) = default;

  any_function& operator=(any_function const&) = default;

  any_function& operator=(any_function&&) = default;

  template <typename F>
  ::std::enable_if_t<
    !::std::is_same<::std::decay_t<F>, any_function>{},
    any_function&
  >
  operator=(F&& f)
  {
    assign(::std::forward<F>(f));

    return *this;
  }

  explicit operator bool() const noexcept
  {
    return bool(any_);
  }

  bool empty() const noexcept
  {
    return !*this;
  }

  template <typename F>
  void assign(F&& f)
  {
    any_ = ::std::forward<F>(f);

    assign<F>(extract_signature(::std::forward<F>(f)));
  }

  template <typename ...A>
  void operator()(A&& ...args)
  {
#ifndef NDEBUG
    assert(type_id<::generic::many<arg_type_t<A>...> >() == type_id_);
#endif // NDEBUG

    auto const a(
      ::generic::many<arg_type_t<A>...>{::std::forward<A>(args)...}
    );

    f_(any_, &a, nullptr);
  }

  template <typename R, typename ...A>
  R operator()(A&& ...args)
  {
#ifndef NDEBUG
    assert(type_id<::generic::many<arg_type_t<A>...> >() == type_id_);
#endif // NDEBUG

    auto const a(
      ::generic::many<arg_type_t<A>...>{::std::forward<A>(args)...}
    );

    alignas(R) char r[sizeof(R)];

    f_(any_, &a, r);

    return *reinterpret_cast<R*>(r);
  }

  template <typename ...A>
  void apply(::generic::many<A...> const& m)
  {
#ifndef NDEBUG
    assert(type_id<::generic::many<A...> >() == type_id_);
#endif // NDEBUG

    f_(any_, &m, nullptr);
  }

  template <typename R, typename ...A>
  R apply(::generic::many<A...> const& m)
  {
#ifndef NDEBUG
    assert(type_id<::generic::many<A...> >() == type_id_);
#endif // NDEBUG

    alignas(R) char r[sizeof(R)];

    f_(any_, &m, r);

    return *reinterpret_cast<R*>(r);
  }
};

}

#endif // GENERIC_ANYFUNCTION_HPP
