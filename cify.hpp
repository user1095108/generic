#ifndef GNR_CIFY_HPP
# define GNR_CIFY_HPP
# pragma once

#include <new>

#include <utility>

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

//////////////////////////////////////////////////////////////////////////////
template <int I, typename F, typename R, typename ...A>
inline auto cify(F&& f, signature<R(A...)>) noexcept
{
  static F f_(std::forward<F>(f));
  static bool full;

  if (full)
  {
    f_.~F();

    new (static_cast<void*>(&f_)) F(std::forward<F>(f));
  }
  else
  {
    full = true;
  }

  return +[](A... args) noexcept(noexcept(
      std::declval<F>()(std::forward<A>(args)...)))
    {
      return f_(std::forward<A>(args)...);
    };
}

//////////////////////////////////////////////////////////////////////////////
template <int I, typename F, typename R, typename ...A>
inline auto cify_once(F&& f, signature<R(A...)>) noexcept
{
  static F f_(std::forward<F>(f));

  return +[](A... args) noexcept(noexcept(
      std::declval<F>()(std::forward<A>(args)...)))
    {
      return f_(std::forward<A>(args)...);
    };
}

}

//////////////////////////////////////////////////////////////////////////////
template <int I = 0, typename F>
auto cify(F&& f) noexcept
{
  return cify<I>(std::forward<F>(f),
    extract_signature(std::forward<F>(f))
  );
}

//////////////////////////////////////////////////////////////////////////////
template <int I = 0, typename F>
auto cify_once(F&& f) noexcept
{
  return cify_once<I>(std::forward<F>(f),
    extract_signature(std::forward<F>(f))
  );
}

}

#endif // GNR_CIFY_HPP
