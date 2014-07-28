#ifndef GENERIC_MEMBER_HPP
# define GENERIC_MEMBER_HPP
# pragma once

#include <type_traits>

#include <utility>

namespace generic
{

#ifndef MEMBER
# define MEMBER(f) decltype(&f),&f
#endif // MEMBER

namespace detail
{

template <typename FP, FP fp, class C, typename ...A>
struct member_delegate_
{
  C* const object;

  auto operator()(A&& ...args) const ->
    decltype((object->*fp)(::std::forward<A>(args)...))
  {
    return (object->*fp)(::std::forward<A>(args)...);
  }
};

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C const* const object, R (C::* const)(A...) const) ->
  member_delegate_<FP, fp, C const, A...>
{
  return {object};
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C const* const object, R (C::* const)(A...) const volatile) ->
  member_delegate_<FP, fp, C const, A...>
{
  return {object};
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C* const object, R (C::* const)(A...) volatile) ->
  member_delegate_<FP, fp, C, A...>
{
  return {object};
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C* const object, R (C::* const)(A...)) ->
  member_delegate_<FP, fp, C, A...>
{
  return {object};
}

}

template <typename FP, FP fp, class C>
inline auto member(C* const object) ->
  decltype(detail::member_delegate<FP, fp>(object, fp))
{
  return detail::member_delegate<FP, fp>(object, fp);
}

template <typename FP, FP fp, class C>
inline auto member(C const* const object) ->
  decltype(detail::member_delegate<FP, fp>(object, fp))
{
  return detail::member_delegate<FP, fp>(object, fp);
}

template <typename FP, FP fp, class C>
inline auto member(C& object) ->
  decltype(detail::member_delegate<FP, fp>(&object, fp))
{
  return detail::member_delegate<FP, fp>(&object, fp);
}

template <typename FP, FP fp, class C>
inline auto member(C const& object) ->
  decltype(detail::member_delegate<FP, fp>(&object, fp))
{
  return detail::member_delegate<FP, fp>(&object, fp);
}

}

#endif // GENERIC_MEMBER_HPP
