#ifndef LIGHTDELEGATE_HPP
# define LIGHTDELEGATE_HPP
# pragma once

#include <utility>

namespace generic
{

#ifndef DELEGATE
# define DELEGATE(f) decltype(&f),&f
#endif // DELEGATE

namespace detail
{
  // stubs
  template <typename FP, FP fp, class R, class ...A>
  static R function_stub(A&&... args)
  {
    return fp(::std::forward<A>(args)...);
  }

  template <typename FP, FP fp, class C, class R, class ...A>
  static R member_function_stub(C* const object, A&&... args)
  {
    return (object->*fp)(::std::forward<A>(args)...);
  }

  // factories
  template <typename FP, FP fp, class R, class ...A>
  inline auto make_delegate(R (* const)(A...)) noexcept ->
    decltype(&function_stub<FP, fp, R, A...>)
  {
    return function_stub<FP, fp, R, A...>;
  }

  template <typename FP, FP fp, class C, class R, class ...A>
  inline auto make_delegate(R (C::* const)(A...)) noexcept ->
    decltype(&member_function_stub<FP, fp, C, R, A...>)
  {
    return member_function_stub<FP, fp, C, R, A...>;
  }

  template <typename FP, FP fp, class C, class R, class ...A>
  inline auto make_delegate(R (C::* const)(A...) const) noexcept ->
    decltype(&member_function_stub<FP, fp, C, R, A...>)
  {
    return member_function_stub<FP, fp, C, R, A...>;
  }
}

template <typename FP, FP fp>
inline auto make_delegate() noexcept ->
  decltype(detail::make_delegate<FP, fp>(fp))
{
  return detail::make_delegate<FP, fp>(fp);
}

}

#endif // LIGHTDELEGATE_HPP
