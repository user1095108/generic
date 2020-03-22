#ifndef GNR_CALLBACK_HPP
# define GNR_CALLBACK_HPP
# pragma once

#include <cassert>

// std::size_t
#include <cstddef>

#include <tuple>

#include <functional>

#include <type_traits>

#include <utility>

namespace gnr
{

namespace detail
{

namespace callback
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
constexpr auto extract_signature(F* const) noexcept
{
  return signature<typename remove_cv_seq<F>::type>();
}

template <typename C, typename F>
constexpr auto extract_signature(F C::* const) noexcept
{
  return signature<typename remove_cv_seq<F>::type>();
}

template <typename F>
constexpr auto extract_signature(F const&) noexcept ->
  decltype(&F::operator(), extract_signature(&F::operator()))
{
  return extract_signature(&F::operator());
}

}

}

template <std::size_t N = detail::callback::default_size>
class callback
{
  using typeid_t = void(*)();

  template <typename T>
  static typeid_t type_id() noexcept
  {
    return typeid_t(type_id<T>);
  }

  void (*f_)(void*, void const*, void*) {};

  std::aligned_storage_t<N> store_;

#ifndef NDEBUG
  typeid_t type_id_;
#endif // NDEBUG

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    !std::is_member_function_pointer<F>{} &&
    !std::is_void<R>{}
  >
  invoker(void* const store, void const* const v, void* const r)
  {
    *static_cast<R*>(r) = std::apply(*static_cast<F*>(store),
      *static_cast<std::tuple<A...> const*>(v)
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    !std::is_member_function_pointer<F>{} &&
    std::is_void<R>{}
  >
  invoker(void* const store, void const* const v, void*)
  {
    std::apply(*static_cast<F*>(store),
      *static_cast<std::tuple<A...> const*>(v)
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    std::is_member_function_pointer<F>{} &&
    !std::is_void<R>{}
  >
  invoker(void* const store, void const* const v, void* const r)
  {
    *static_cast<R*>(r) = std::apply(*static_cast<F*>(store),
      *static_cast<
        std::tuple<detail::callback::class_ref_t<F>, A...> const*
      >(v)
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    std::is_member_function_pointer<F>{} &&
    std::is_void<R>{}
  >
  invoker(void* const store, void const* const v, void*)
  {
    std::apply(*static_cast<F*>(store),
      *static_cast<
        std::tuple<detail::callback::class_ref_t<F>, A...> const*
      >(v)
    );
  }

  template <typename F, typename R, typename ...A>
  std::enable_if_t<!std::is_member_function_pointer<F>{}>
  assign(detail::callback::signature<R(A...)>) noexcept
  {
    f_ = invoker<std::decay_t<F>, R, A...>;

#ifndef NDEBUG
    type_id_ = type_id<std::tuple<R, A...>>();
#endif // NDEBUG
  }

  template <typename F, typename R, typename ...A>
  std::enable_if_t<std::is_member_function_pointer<F>{}>
  assign(detail::callback::signature<R(A...)>) noexcept
  {
    f_ = invoker<F, R, A...>;

#ifndef NDEBUG
    type_id_ = type_id<
      std::tuple<R, detail::callback::class_ref_t<F>, A...>
    >();
#endif // NDEBUG
  }

public:
  enum : std::size_t { size = N };

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

  callback() = default;

  callback(callback const&) = default;

  callback(callback&&) = default;

  template <typename F, typename =
    std::enable_if_t<!std::is_same<std::decay_t<F>, callback>{}>
  >
  callback(F&& f) noexcept
  {
    assign(std::forward<F>(f));
  }

  bool operator==(std::nullptr_t) const noexcept
  {
    return f_;
  }

  bool operator!=(std::nullptr_t) const noexcept
  {
    return !operator==(nullptr);
  }

  callback& operator=(callback const&) = default;

  callback& operator=(callback&&) = default;

  template <typename F, typename =
    std::enable_if_t<!std::is_same<std::decay_t<F>, callback>{}>
  >
  callback& operator=(F&& f) noexcept
  {
    assign(std::forward<F>(f));

    return *this;
  }

  explicit operator bool() const noexcept { return f_; }
  template <typename R, typename ...A>
  std::enable_if_t<!std::is_void<R>{}, R>
  operator()(A&&... args) const noexcept(
    noexcept(std::declval<callback>().template invoke<R>(
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
    noexcept(std::declval<callback>().template invoke<R>(
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
  void assign(F&& f) noexcept
  {
    using functor_type = std::decay_t<F>;

    static_assert(sizeof(functor_type) <= sizeof(store_),
      "functor too large");
    static_assert(std::is_trivially_copyable<functor_type>{},
      "functor not trivially copyable");

    ::new (static_cast<void*>(&store_)) functor_type(std::forward<F>(f));

    assign<F>(detail::callback::extract_signature(std::forward<F>(f)));
  }

  template <typename R, typename ...A>
  std::enable_if_t<!std::is_void<R>{}, R>
  invoke(A... args) const
    noexcept(
        noexcept(f_(const_cast<void*>(static_cast<void const*>(&store_)),
          {},
          {}
        )
      )
    )
  {
    assert(f_);
#ifndef NDEBUG
    using test_t = std::tuple<R, arg_type_t<A>...>;
    assert(type_id<test_t>() == type_id_);
#endif // NDEBUG

    R r;

    auto const a(std::tuple<arg_type_t<A>...>{std::forward<A>(args)...});

    f_(const_cast<void*>(static_cast<void const*>(&store_)), &a, &r);

    return r;
  }

  template <typename R = void, typename ...A>
  std::enable_if_t<std::is_void<R>{}, R>
  invoke(A... args) const
    noexcept(
        noexcept(f_(const_cast<void*>(static_cast<void const*>(&store_)),
          {},
          {}
        )
      )
    )
  {
    assert(f_);
#ifndef NDEBUG
    using test_t = std::tuple<R, arg_type_t<A>...>;
    assert(type_id<test_t>() == type_id_);
#endif // NDEBUG

    auto const a(std::tuple<arg_type_t<A>...>{std::forward<A>(args)...});

    f_(const_cast<void*>(static_cast<void const*>(&store_)), &a, {});
  }

  void reset() noexcept { f_ = {}; }

  void swap(callback& other) noexcept
  {
    std::swap(*this, other);
  }

  void swap(callback&& other) noexcept
  {
    std::swap(*this, std::move(other));
  }

  template <typename T>
  auto target() noexcept
  {
    return reinterpret_cast<T*>(&store_);
  }

  template <typename T> 
  auto target() const noexcept
  {
    return reinterpret_cast<T const*>(&store_);
  }
};

}

#endif // GNR_CALLBACK_HPP
