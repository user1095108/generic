#ifndef GENERIC_MEMFUN_HPP
# define GENERIC_MEMFUN_HPP
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

namespace mem_fun
{

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C* const object,
  R (C::* const)(A...) const) noexcept
{
  return [object](A&& ...args) noexcept(
      noexcept((object->*fp)(::std::forward<A>(args)...))
    )
    {
      return (object->*fp)(::std::forward<A>(args)...);
    };
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C* const object,
  R (C::* const)(A...) const volatile) noexcept
{
  return [object](A&& ...args) noexcept(
      noexcept((object->*fp)(::std::forward<A>(args)...))
    )
    {
      return (object->*fp)(::std::forward<A>(args)...);
    };
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C* const object,
  R (C::* const)(A...) volatile) noexcept
{
  return [object](A&& ...args) noexcept(
      noexcept((object->*fp)(::std::forward<A>(args)...))
    )
    {
      return (object->*fp)(::std::forward<A>(args)...);
    };
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C* const object,
  R (C::* const)(A...)) noexcept
{
  return [object](A&& ...args) noexcept(
      noexcept((object->*fp)(::std::forward<A>(args)...))
    )
    {
      return (object->*fp)(::std::forward<A>(args)...);
    };
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C& object,
  R (C::* const)(A...) const) noexcept
{
  return [&object](A&& ...args) noexcept(
      noexcept((object.*fp)(::std::forward<A>(args)...))
    )
    {
      return (object.*fp)(::std::forward<A>(args)...);
    };
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C& object,
  R (C::* const)(A...) const volatile) noexcept
{
  return [&object](A&& ...args) noexcept(
      noexcept((object.*fp)(::std::forward<A>(args)...))
    )
    {
      return (object.*fp)(::std::forward<A>(args)...);
    };
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C& object,
  R (C::* const)(A...) volatile) noexcept
{
  return [&object](A&& ...args) noexcept(
      noexcept((object.*fp)(::std::forward<A>(args)...))
    )
    {
      return (object.*fp)(::std::forward<A>(args)...);
    };
}

template <typename FP, FP fp, typename R, class C, typename ...A>
inline auto member_delegate(C& object,
  R (C::* const)(A...)) noexcept
{
  return [&object](A&& ...args) noexcept(
      noexcept((object.*fp)(::std::forward<A>(args)...))
    )
    {
      return (object.*fp)(::std::forward<A>(args)...);
    };
}

}

}

template <typename FP, FP fp, class C>
inline auto mem_fun(C* const object) noexcept
{
  return detail::mem_fun::member_delegate<FP, fp>(object, fp);
}

template <typename FP, FP fp, class C>
inline auto mem_fun(C& object) noexcept
{
  return detail::mem_fun::member_delegate<FP, fp>(object, fp);
}

}

#endif // GENERIC_MEMFUN_HPP
