#ifndef GNR_MEMFUN_HPP
# define GNR_MEMFUN_HPP
# pragma once

#include <utility>

#define MEM_FUN(f) decltype(&f),&f

namespace gnr
{

namespace
{

#define GNR_GEN_MEM_DELEGATE(OBJECT, MEMFUN, CAPTURE, CALL)	   \
template <typename FP, FP fp, typename R, class C, typename ...A>  \
inline auto member_delegate(OBJECT, MEMFUN) noexcept               \
{                                                                  \
  return [CAPTURE](A&& ...args) noexcept(                          \
      noexcept((CALL)(::std::forward<A>(args)...))                 \
    )                                                              \
    {                                                              \
      return (CALL)(::std::forward<A>(args)...);                   \
    };                                                             \
}

GNR_GEN_MEM_DELEGATE(C* const       object, R (C::* const)(A...) const,           object, object->*fp)
GNR_GEN_MEM_DELEGATE(C* const       object, R (C::* const)(A...) const volatile,  object, object->*fp)
GNR_GEN_MEM_DELEGATE(C* const       object, R (C::* const)(A...)       volatile,  object, object->*fp)
GNR_GEN_MEM_DELEGATE(C* const       object, R (C::* const)(A...),                 object, object->*fp)

GNR_GEN_MEM_DELEGATE(const C* const object, R (C::* const)(A...) const,           object, object->*fp)
GNR_GEN_MEM_DELEGATE(const C* const object, R (C::* const)(A...) const volatile,  object, object->*fp)
GNR_GEN_MEM_DELEGATE(const C* const object, R (C::* const)(A...)       volatile,  object, object->*fp)
GNR_GEN_MEM_DELEGATE(const C* const object, R (C::* const)(A...),                 object, object->*fp)

GNR_GEN_MEM_DELEGATE(C&             object, R (C::* const)(A...) const,          &object, object.*fp)
GNR_GEN_MEM_DELEGATE(C&             object, R (C::* const)(A...) const volatile, &object, object.*fp)
GNR_GEN_MEM_DELEGATE(C&             object, R (C::* const)(A...)       volatile, &object, object.*fp)
GNR_GEN_MEM_DELEGATE(C&             object, R (C::* const)(A...),                &object, object.*fp)

GNR_GEN_MEM_DELEGATE(const       C& object, R (C::* const)(A...) const,          &object, object.*fp)
GNR_GEN_MEM_DELEGATE(const       C& object, R (C::* const)(A...) const volatile, &object, object.*fp)
GNR_GEN_MEM_DELEGATE(const       C& object, R (C::* const)(A...)       volatile, &object, object.*fp)
GNR_GEN_MEM_DELEGATE(const       C& object, R (C::* const)(A...),                &object, object.*fp)
    
}

template <typename FP, FP fp, class C>
inline auto mem_fun(C* const object) noexcept
{
  return member_delegate<FP, fp>(object, fp);
}

template <typename FP, FP fp, class C>
inline auto mem_fun(const C* const object) noexcept
{
  return member_delegate<FP, fp>(object, fp);
}

template <typename FP, FP fp, class C>
inline auto mem_fun(C& object) noexcept
{
  return member_delegate<FP, fp>(object, fp);
}

template <typename FP, FP fp, class C>
inline auto mem_fun(const C& object) noexcept
{
  return member_delegate<FP, fp>(object, fp);
}

}

#endif // GNR_MEMFUN_HPP
