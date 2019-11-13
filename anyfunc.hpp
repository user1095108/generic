#ifndef GNR_ANYFUNC_HPP
# define GNR_ANYFUNC_HPP
# pragma once

#include <cassert>

// std::size_t
#include <cstddef>

#include <any>

#include <tuple>

#include <functional>

#include <type_traits>

#include <utility>

namespace gnr
{

namespace detail
{

namespace anyfunc
{

constexpr auto default_size = 4 * sizeof(void*);

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

}

}

template <typename Any = std::any>
class anyfunc
{
  using typeid_t = void(*)();

  template <typename T>
  static typeid_t type_id() noexcept
  {
    return typeid_t(type_id<T>);
  }

  void (*f_)(Any const&, void const*, void*) {};

  Any any_;

#ifndef NDEBUG
  typeid_t type_id_;
#endif // NDEBUG

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    !std::is_member_function_pointer<F>{} &&
    !std::is_void<R>{}
  >
  invoker(Any const& any, void const* const v, void* const r)
  {
    *static_cast<R*>(r) = std::apply(std::any_cast<F>(any),
      *static_cast<std::tuple<A...> const*>(v)
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    !std::is_member_function_pointer<F>{} &&
    std::is_void<R>{}
  >
  invoker(Any const& any, void const* const v, void*)
  {
    std::apply(std::any_cast<F>(any),
      *static_cast<std::tuple<A...> const*>(v)
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    std::is_member_function_pointer<F>{} &&
    !std::is_void<R>{}
  >
  invoker(Any const& any, void const* const v, void* const r)
  {
    *static_cast<R*>(r) = std::apply(std::any_cast<F>(any),
      *static_cast<
        std::tuple<detail::anyfunc::class_ref_t<F>, A...> const*
      >(v)
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    std::is_member_function_pointer<F>{} &&
    std::is_void<R>{}
  >
  invoker(Any const& any, void const* const v, void*)
  {
    std::apply(std::any_cast<F>(any),
      *static_cast<
        std::tuple<detail::anyfunc::class_ref_t<F>, A...> const*
      >(v)
    );
  }

  template <typename F, typename R, typename ...A>
  std::enable_if_t<!std::is_member_function_pointer<F>{}>
  assign(detail::anyfunc::signature<R(A...)>) noexcept
  {
    f_ = invoker<std::decay_t<F>, R, A...>;

#ifndef NDEBUG
    type_id_ = type_id<std::tuple<R, A...>>();
#endif // NDEBUG
  }

  template <typename F, typename R, typename ...A>
  std::enable_if_t<std::is_member_function_pointer<F>{}>
  assign(detail::anyfunc::signature<R(A...)>) noexcept
  {
    f_ = invoker<F, R, A...>;

#ifndef NDEBUG
    type_id_ = type_id<
      std::tuple<R, detail::anyfunc::class_ref_t<F>, A...>
    >();
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

  anyfunc() = default;

  template <typename F,
    typename = std::enable_if_t<
      !std::is_same<std::decay_t<F>, anyfunc>{}
    >
  >
  anyfunc(F&& f) :
    any_(std::forward<F>(f))
  {
    assign<F>(detail::anyfunc::extract_signature(std::forward<F>(f)));
  }

  anyfunc(anyfunc const&) = default;

  anyfunc(anyfunc&&) = default;

  anyfunc& operator=(anyfunc const&) = default;

  anyfunc& operator=(anyfunc&&) = default;

  template <typename F,
    typename = std::enable_if_t<
      !std::is_same<std::decay_t<F>, anyfunc>{}
    >
  >
  anyfunc& operator=(F&& f)
  {
    assign(std::forward<F>(f));

    return *this;
  }

  bool operator==(std::nullptr_t) const noexcept
  {
    return f_;
  }

  bool operator!=(std::nullptr_t) const noexcept
  {
    return !operator==(nullptr);
  }

  explicit operator bool() const noexcept
  {
    return any_.has_value();
  }

  template <typename R, typename ...A>
  std::enable_if_t<!std::is_void<R>{}, R>
  operator()(A&&... args) const noexcept(
    noexcept(std::declval<anyfunc>().template invoke<R>(
        std::forward<A>(args)...
      )
    )
  )
  {
    return invoke<R>(std::forward<A>(args)...);
  }

  template <typename R = void, typename ...A>
  std::enable_if_t<std::is_void<R>{}, R>
  operator()(A&&... args) const noexcept(
    noexcept(std::declval<anyfunc>().template invoke<R>(
        std::forward<A>(args)...
      )
    )
  )
  {
    invoke<R>(std::forward<A>(args)...);
  }

  void assign(std::nullptr_t) noexcept
  {
    reset();
  }

  template <typename F>
  void assign(F&& f)
  {
    any_ = std::forward<F>(f);

    assign<F>(detail::anyfunc::extract_signature(std::forward<F>(f)));
  }

  bool empty() const noexcept
  {
    return !*this;
  }

  bool has_value() const noexcept
  {
    return *this;
  }

  template <typename R, typename ...A>
  std::enable_if_t<!std::is_void<R>{}, R>
  invoke(A... args) const noexcept(noexcept(f_(any_, {}, {})))
  {
    assert(f_);
#ifndef NDEBUG
    using test_t = std::tuple<R, arg_type_t<A>...>;
    assert(type_id<test_t>() == type_id_);
#endif // NDEBUG

    R r;

    auto const a(std::tuple<arg_type_t<A>...>{std::forward<A>(args)...});

    f_(any_, &a, &r);

    return r;
  }

  template <typename R = void, typename ...A>
  std::enable_if_t<std::is_void<R>{}, R>
  invoke(A... args) const noexcept(noexcept(f_(any_, {}, {})))
  {
    assert(f_);
#ifndef NDEBUG
    using test_t = std::tuple<R, arg_type_t<A>...>;
    assert(type_id<test_t>() == type_id_);
#endif // NDEBUG

    auto const a(std::tuple<arg_type_t<A>...>{std::forward<A>(args)...});

    f_(any_, &a, {});
  }

  void reset() noexcept { f_ = {}; }

  void swap(anyfunc& other) noexcept
  {
    std::swap(*this, other);
  }

  void swap(anyfunc&& other) noexcept
  {
    std::swap(*this, std::move(other));
  }
};

}

#endif // GNR_ANYFUNC_HPP
