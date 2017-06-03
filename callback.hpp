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

namespace
{

constexpr auto default_noexcept =
#if defined(__cpp_exceptions) && __cpp_exceptions
false;
#else
true;
#endif // __cpp_exceptions

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

template <std::size_t N = default_size, bool NE = default_noexcept>
class callback<N, NE>
{
  using typeid_t = void(*)();

  template <typename T>
  static typeid_t type_id() noexcept
  {
    return typeid_t(type_id<T>);
  }

  void (*f_)(void const*) noexcept(NE) {};

  std::aligned_storage_t<N> store_;

#ifndef NDEBUG
  typeid_t type_id_;
#endif // NDEBUG

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    !std::is_member_function_pointer<F>{} &&
    !std::is_void<R>{}
  >
  invoker(void* const store,
    void const* const v,
    R* const r) noexcept(
    noexcept(do_invoke<R, A...>(std::get<F>(any),
      *static_cast<std::tuple<A...> const*>(v),
      std::make_index_sequence<sizeof...(A)>())
    )
  )
  {
    *r = std::apply(*static_cast<F*>(store),
      *static_cast<std::tuple<A...> const*>(v),
      std::make_index_sequence<sizeof...(A)>()
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    !std::is_member_function_pointer<F>{} &&
    std::is_void<R>{}
  >
  invoker(void* const store,
    void const* const v,
    void*) noexcept(
    noexcept(std::apply(*static_cast<F*>(store),
        *static_cast<std::tuple<A...> const*>(v),
        std::make_index_sequence<sizeof...(A)>()
      )
    )
  )
  {
    std::apply(*static_cast<F*>(store),
      *static_cast<std::tuple<A...> const*>(v),
      std::make_index_sequence<sizeof...(A)>()
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    std::is_member_function_pointer<F>{} &&
    !is_void<R>{}
  >
  invoker(void* const store,
    void const* const v,
    R* const r) noexcept(
    noexcept(std::apply(
        *static_cast<F*>(store),
        *static_cast<std::tuple<class_ref_t<F>, A...> const*>(v)
      )
    )
  )
  {
    *r = std::apply(*static_cast<F*>(store),
      *static_cast<std::tuple<class_ref_t<F>, A...> const*>(v)
    );
  }

  template <typename F, typename R, typename ...A>
  static std::enable_if_t<
    std::is_member_function_pointer<F>{} &&
    is_void<R>{}
  >
  invoker(void* const store,
    void const* const v,
    void*) noexcept(
    noexcept(do_invoke<R, A...>(
      *static_cast<std::tuple<class_ref_t<F>, A...> const*>(v),
      std::make_index_sequence<sizeof...(A) + 1>())
    )
  )
  {
    std::apply(*static_cast<F*>(store),
      *static_cast<std::tuple<class_ref_t<F>, A...> const*>(v)
    );
  }

  template<typename T, typename ...U, std::size_t M>
  friend bool operator==(callback<T (U...), M> const&,
    std::nullptr_t) noexcept;
  template<typename T, typename ...U, std::size_t M>
  friend bool operator==(std::nullptr_t,
    callback<T (U...), M> const&) noexcept;

  template<typename T, typename ...U, std::size_t M>
  friend bool operator!=(callback<T (U...), M> const&,
    std::nullptr_t) noexcept;
  template<typename T, typename ...U, std::size_t M>
  friend bool operator!=(std::nullptr_t,
    callback<T (U...), M> const&) noexcept;

public:
  using result_type = R;

  enum : std::size_t { size = N };

public:
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

  explicit operator bool() const noexcept { return stub_; }

  template <typename R = void>
  R operator()(A... args) const
    noexcept(
        noexcept(stub_(const_cast<void*>(static_cast<void const*>(&store_)),
          std::forward<A>(args)...
        )
      )
    )
  {
    //assert(f_);
    f_(const_cast<void*>(static_cast<void const*>(&store_)),
      std::forward<A>(args)...
    );
  }

  template <typename R>
  std::enable_if_t<!std::is_void<R>{}, R>
  operator()(A... args) const
    noexcept(
        noexcept(f_(const_cast<void*>(static_cast<void const*>(&store_)),
          nullptr,
          std::forward<A>(args)...
        )
      )
    )
  {
    //assert(f_);
    R r;

    f_(const_cast<void*>(static_cast<void const*>(&store_)),
      &r,
      std::forward<A>(args)...
    );

    return r;
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

    stub_ = [](void* const ptr, A&&... args) noexcept(
        noexcept(
        (
#if __cplusplus <= 201402L
          (*static_cast<functor_type*>(ptr))(
            std::forward<A>(args)...)
#else
          std::invoke(*static_cast<functor_type*>(ptr),
            std::forward<A>(args)...)
#endif // __cplusplus
        )
      )
    ) -> R
    {
#if __cplusplus <= 201402L
      return (*static_cast<functor_type*>(ptr))(
        std::forward<A>(args)...);
#else
      return std::invoke(*static_cast<functor_type*>(ptr),
        std::forward<A>(args)...);
#endif // __cplusplus
    };
  }

  void reset() noexcept { stub_ = nullptr; }

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

template <std::size_t N>
bool operator==(callback<N> const& f, std::nullptr_t const) noexcept
{
  return nullptr == f.stub_ ;
}

template<std::size_t N>
bool operator==(std::nullptr_t, callback<N> const& f) noexcept
{
  return nullptr == f.stub_;
}

template<std::size_t N>
bool operator!=(callback<N> const& f, std::nullptr_t const) noexcept
{
  return !operator==(nullptr, f);
}

template<std::size_t N>
bool operator!=(std::nullptr_t, callback<N> const& f) noexcept
{
  return !operator==(nullptr, f);
}

}

#endif // GNR_CALLBACK_HPP
