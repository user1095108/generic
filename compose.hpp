#ifndef COMPOSE_HPP
# define COMPOSE_HPP
# pragma once

#include <utility>

namespace
{

template <typename R, typename ...A>
struct signature
{
};

template <typename R, typename ...A>
constexpr auto extract_signature(R (*const)(A...)) noexcept
{
  return signature<R, A...>();
}

template <typename C, typename R, typename ...A>
constexpr auto extract_signature(R (C::* const)(A...)) noexcept
{
  return signature<R, A...>();
}

template <typename C, typename R, typename ...A>
constexpr auto extract_signature(R (C::* const)(A...) const) noexcept
{
  return signature<R, A...>();
}

template <typename F>
constexpr auto extract_signature(F const&) noexcept ->
  decltype(&F::operator(), extract_signature(&F::operator()))
{
  return extract_signature(&F::operator());
}

template <typename F1, typename F2, typename ...A1>
class composer
{
  F1 const f1_;
  F2 const f2_;

public:
  explicit composer(F1&& f1, F2&& f2) noexcept :
    f1_(::std::forward<F1>(f1)),
    f2_(::std::forward<F2>(f2))
  {
  }

  auto operator()(A1&& ...args) const noexcept(
    noexcept(f2_(f1_(::std::forward<A1>(args)...)))
  )
  {
    return f2_(f1_(::std::forward<A1>(args)...));
  }
};

template <
  typename F1, typename R1, typename ...A1,
  typename F2, typename R2, typename ...A2
>
inline auto compose(
  F1&& f1, signature<R1, A1...> const,
  F2&& f2, signature<R2, A2...> const) noexcept(
  noexcept(
    composer<F1, F2, A1...>(::std::forward<F1>(f1),
      ::std::forward<F2>(f2)
    )
  )
)
{
  return composer<F1, F2, A1...>(::std::forward<F1>(f1),
    ::std::forward<F2>(f2)
  );
}

}

template <typename F1, typename F2>
inline auto operator|(F1&& f1, F2&& f2) noexcept(
  noexcept(
    compose(
      ::std::forward<F1>(f1), extract_signature(f1),
      ::std::forward<F2>(f2), extract_signature(f2)
    )
  )
)
{
  return compose(
    ::std::forward<F1>(f1), extract_signature(f1),
    ::std::forward<F2>(f2), extract_signature(f2)
  );
}

#endif // COMPOSE_HPP
