#ifndef MEMBERDELEGATE_HPP
# define MEMBERDELEGATE_HPP
# pragma once

#include <type_traits>

namespace generic
{

#ifndef DELEGATE
# define DELEGATE(f) decltype(&f),&f
#endif // DELEGATE

namespace detail
{

template <typename T>
auto address(T&& t) -> typename ::std::remove_reference<T>::type*
{
  return &t;
}

template <typename FP, FP fp, class C, typename ...A>
struct S
{
  static constexpr auto* l = false ?
    address([](C* const object) noexcept {
      return [object](A&& ...args) {
        return (object->*fp)(::std::forward<A>(args)...); 
      };
    }) :
    nullptr;
};

template <typename FP, FP fp, typename R, class C, typename ...A>
auto make_delegate(C* const object, R (C::* const)(A...) const) ->
  decltype((*decltype(S<FP, fp, C, A...>::l)(nullptr))(object))
{
  return (*decltype(S<FP, fp, C, A...>::l)(nullptr))(object);
}

template <typename FP, FP fp, typename R, class C, typename ...A>
auto make_delegate(C* const object, R (C::* const)(A...) const volatile) ->
  decltype((*decltype(S<FP, fp, C, A...>::l)(nullptr))(object))
{
  return (*decltype(S<FP, fp, C, A...>::l)(nullptr))(object);
}

template <typename FP, FP fp, typename R, class C, typename ...A>
auto make_delegate(C* const object, R (C::* const)(A...) volatile) ->
  decltype((*decltype(S<FP, fp, C, A...>::l)(nullptr))(object))
{
  return (*decltype(S<FP, fp, C, A...>::l)(nullptr))(object);
}

template <typename FP, FP fp, typename R, class C, typename ...A>
auto make_delegate(C* const object, R (C::* const)(A...)) ->
  decltype((*decltype(S<FP, fp, C, A...>::l)(nullptr))(object))
{
  return (*decltype(S<FP, fp, C, A...>::l)(nullptr))(object);
}

}

template <typename FP, FP fp, class C>
auto make_delegate(C* const object) ->
  decltype(detail::make_delegate<FP, fp>(object, fp))
{
  return detail::make_delegate<FP, fp>(object, fp);
}

}

#endif // MEMBERDELEGATE_HPP
