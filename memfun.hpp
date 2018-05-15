#ifndef GNR_MEMFUN_HPP
# define GNR_MEMFUN_HPP
# pragma once

#include <functional>

#include <type_traits>

#include <utility>

#define MEMFUN(f) decltype(&f),&f

namespace gnr
{

namespace mem_fun
{

namespace detail
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

//
template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) noexcept>
{
  using type = C&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const noexcept>
{
  using type = C const&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const volatile noexcept>
{
  using type = C const volatile&;
};

//
template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) & noexcept>
{
  using type = C&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const & noexcept>
{
  using type = C const&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const volatile & noexcept>
{
  using type = C const volatile&;
};

//
template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) && noexcept>
{
  using type = C&&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const && noexcept>
{
  using type = C const &&;
};

template <typename R, typename C, typename ...A>
struct class_ref<R (C::*)(A...) const volatile && noexcept>
{
  using type = C const volatile &&;
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
struct remove_cv_seq<R(A...) const noexcept>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) volatile noexcept>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const volatile noexcept>
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
struct remove_cv_seq<R(A...) & noexcept>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const & noexcept >
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) volatile & noexcept>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const volatile & noexcept>
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

//
template <typename R, typename ...A>
struct remove_cv_seq<R(A...) && noexcept>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const && noexcept>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) volatile && noexcept>
{
  using type = R(A...);
};

template <typename R, typename ...A>
struct remove_cv_seq<R(A...) const volatile && noexcept>
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

template <typename FP, FP fp, typename REF, typename R, typename ...A>
inline auto member_delegate(REF& ref, signature<R(A...)>) noexcept
{
  return [&ref](A ...args) noexcept(
    noexcept(std::invoke(fp, ref, std::forward<A>(args)...))
  )
  {
    return std::invoke(fp, ref, std::forward<A>(args)...);
  };
}

template <typename FP, FP fp, typename REF, typename R, typename ...A>
inline auto member_delegate(REF* ref, signature<R(A...)>) noexcept
{
  return [ref](A ...args) noexcept(
    noexcept(std::invoke(fp, ref, std::forward<A>(args)...))
  )
  {
    return std::invoke(fp, ref, std::forward<A>(args)...);
  };
}

template <typename FP, FP fp, typename R, typename ...A>
inline auto member_delegate_ref(signature<R(A...)>) noexcept
{
  return [](class_ref_t<FP> ref, A ...args) noexcept(
    noexcept(std::invoke(fp, ref, std::forward<A>(args)...))
  )
  {
    return std::invoke(fp, ref, std::forward<A>(args)...);
  };
}

}

}

template <typename FP, FP fp, typename REF,
  typename = std::enable_if_t<std::is_member_function_pointer<FP>{} ||
    std::is_reference<REF>{} || std::is_pointer<REF>{}
  >
>
inline auto memfun(REF&& ref) noexcept
{
  return mem_fun::detail::member_delegate<FP, fp>(std::forward<REF>(ref),
    mem_fun::detail::extract_signature(fp));
}

template <typename FP, FP fp,
  typename = std::enable_if_t<std::is_member_function_pointer<FP>{}>
>
inline auto memfun_ref() noexcept
{
  return mem_fun::detail::member_delegate_ref<FP, fp>(
    mem_fun::detail::extract_signature(fp));
}

}

#endif // GNR_MEMFUN_HPP
