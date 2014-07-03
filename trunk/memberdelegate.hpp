#ifndef MEMBERDELEGATE_HPP
# define MEMBERDELEGATE_HPP
# pragma once

#include <type_traits>

namespace generic
{

#ifndef MEMBER
# define MEMBER(f) decltype(&f),&f
#endif // MEMBER

namespace detail
{

template <typename FP, FP fp, class C, typename ...A>
struct member_delegate
{
  C* const object;

  auto operator()(A&& ...args) const ->
    decltype((object->*fp)(::std::forward<A>(args)...))
  {
    return (object->*fp)(::std::forward<A>(args)...);
  }
};

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto make_delegate(C* const object, R (C::* const)(A...) const) ->
  member_delegate<FP, fp, C, A...>
{
  return {object};
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto make_delegate(C* const object, R (C::* const)(A...) const volatile) ->
  member_delegate<FP, fp, C, A...>
{
  return {object};
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto make_delegate(C* const object, R (C::* const)(A...) volatile) ->
  member_delegate<FP, fp, C, A...>
{
  return {object};
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto make_delegate(C* const object, R (C::* const)(A...)) ->
  member_delegate<FP, fp, C, A...>
{
  return {object};
}

}

template <typename FP, FP fp, class C>
auto make_delegate(C* const object) ->
  decltype(detail::make_delegate<FP, fp>(object, fp))
{
  return detail::make_delegate<FP, fp>(object, fp);
}

template <typename FP, FP fp, class C>
auto make_delegate(C& object) ->
  decltype(detail::make_delegate<FP, fp>(&object, fp))
{
  return detail::make_delegate<FP, fp>(&object, fp);
}

template <typename FP, FP fp, class C>
auto make_delegate(C const* const object) ->
  decltype(detail::make_delegate<FP, fp>(object, fp))
{
  return detail::make_delegate<FP, fp>(object, fp);
}

template <typename FP, FP fp, class C>
auto make_delegate(C const& object) ->
  decltype(detail::make_delegate<FP, fp>(&object, fp))
{
  return detail::make_delegate<FP, fp>(&object, fp);
}

}

#endif // MEMBERDELEGATE_HPP
