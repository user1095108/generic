#ifndef GNR_ANYFUNCTION_HPP
# define GNR_ANYFUNCTION_HPP
# pragma once

#include <cassert>

#include <cstdint>

#include <functional>

#include "many.hpp"

#include "some.hpp"

namespace gnr
{

namespace
{

template <typename>
struct signature
{
};

//
template <typename>
struct class_ref;

//
template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...)>
{
  using type = C&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const>
{
  using type = C const&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const volatile>
{
  using type = C const volatile&;
};

//
template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) &>
{
  using type = C&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const &>
{
  using type = C const&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const volatile &>
{
  using type = C const volatile&;
};

//
template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) &&>
{
  using type = C&&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const &&>
{
  using type = C const&&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const volatile &&>
{
  using type = C const volatile&&;
};

template <typename F>
using class_ref_t = typename class_ref<F>::type;

//
template <typename>
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

template <typename Any = gnr::some<4 * sizeof(void*)>>
class any_function
{
  using typeid_t = std::uintptr_t;

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

  void (*f_)(Any const&, void const*, Any&);

  Any any_;

#ifndef NDEBUG
  typeid_t type_id_;
#endif // NDEBUG

  template <typename R, typename ...A, typename F, std::size_t ...I>
  static std::enable_if_t<!std::is_void<R>{}>
  do_invoke(F&& f,
    gnr::many<A...> const& t,
    Any& r,
    std::index_sequence<I...> const) noexcept(
#if __cplusplus <= 201402L
      noexcept(f(std::get<I>(t)...))
#else
      noexcept(std::invoke(f, std::get<I>(t)...))
#endif
    )
  {
#if __cplusplus <= 201402L
    r = f(std::get<I>(t)...);
#else
    r = std::invoke(f, std::get<I>(t)...);
#endif
  }

  template <typename R, typename ...A, typename F, std::size_t ...I>
  static std::enable_if_t<std::is_void<R>{}>
  do_invoke(F&& f,
    gnr::many<A...> const& t,
    Any&,
    std::index_sequence<I...> const) noexcept(
#if __cplusplus <= 201402L
      noexcept(f(std::get<I>(t)...))
#else
      noexcept(std::invoke(f, std::get<I>(t)...))
#endif
    )
  {
#if __cplusplus <= 201402L
    f(std::get<I>(t)...);
#else
    std::invoke(f, std::get<I>(t)...);
#endif
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<!std::is_member_function_pointer<F>{}>
  invoker(Any const& any,
    void const* const v,
    Any& r) noexcept(
    noexcept(do_invoke<R, A...>(std::get<F>(any),
      *static_cast<gnr::many<A...> const*>(v),
      r,
      std::make_index_sequence<sizeof...(A)>())
    )
  )
  {
    do_invoke<R, A...>(std::get<F>(any),
      *static_cast<gnr::many<A...> const*>(v),
      r,
      std::make_index_sequence<sizeof...(A)>()
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<std::is_member_function_pointer<F>{}>
  invoker(Any const& any,
    void const* const v,
    Any& r) noexcept(
    noexcept(do_invoke<R, A...>(std::get<F>(any),
      *static_cast<gnr::many<class_ref_t<F>, A...> const*>(v),
      r,
      std::make_index_sequence<sizeof...(A) + 1>())
    )
  )
  {
    do_invoke<R, A...>(std::get<F>(any),
      *static_cast<gnr::many<class_ref_t<F>, A...> const*>(v),
      r,
      std::make_index_sequence<sizeof...(A) + 1>()
    );
  }

  template <typename F, typename R, typename ...A>
  std::enable_if_t<!std::is_member_function_pointer<F>{}>
  assign(signature<R(A...)>) noexcept
  {
    f_ = invoker<F, R, A...>;

#ifndef NDEBUG
    type_id_ = type_id<gnr::many<A...>>();
#endif // NDEBUG
  }

  template <typename F, typename R, typename ...A>
  std::enable_if_t<std::is_member_function_pointer<F>{}>
  assign(signature<R(A...)>) noexcept
  {
    f_ = invoker<F, R, A...>;

#ifndef NDEBUG
    type_id_ = type_id<gnr::many<class_ref_t<F>, A...>>();
#endif // NDEBUG
  }

public:
  template <typename T>
  struct arg_type
  {
    using type = std::decay_t<T>;
  };

  template <typename T>
  struct arg_type<std::reference_wrapper<T> >
  {
    using type = T&;
  };

  template <typename T>
  using arg_type_t = typename arg_type<T>::type;

  any_function() = default;

  template <typename F,
    typename = std::enable_if_t<
      !std::is_same<std::decay_t<F>, any_function>{}
    >
  >
  any_function(F&& f) :
    any_(std::forward<F>(f))
  {
    assign<F>(extract_signature(std::forward<F>(f)));
  }

  any_function(any_function const&) = default;

  any_function(any_function&&) = default;

  any_function& operator=(any_function const&) = default;

  any_function& operator=(any_function&&) = default;

  template <typename F,
    typename = std::enable_if_t<
      !std::is_same<std::decay_t<F>, any_function>{}
    >
  >
  any_function& operator=(F&& f)
  {
    assign(std::forward<F>(f));

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
    any_ = std::forward<F>(f);

    assign<F>(extract_signature(std::forward<F>(f)));
  }

  template <typename ...A>
  auto operator()(A&& ...args) const
  {
    return invoke(std::forward<A>(args)...);
  }

  template <typename ...A>
  Any apply(gnr::many<A...> const& m) const
  {
#ifndef NDEBUG
    assert(type_id<gnr::many<A...> >() == type_id_);
#endif // NDEBUG

    Any result;

    f_(any_, &m, result);

    return result;
  }

  template <typename ...A>
  auto invoke(A&& ...args) const
  {
    return apply(gnr::many<arg_type_t<A>...>{std::forward<A>(args)...});
  }
};

}

#endif // GNR_ANYFUNCTION_HPP
