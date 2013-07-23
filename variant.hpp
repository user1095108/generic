#pragma once
#ifndef VARIANT_HPP
# define VARIANT_HPP

#include <type_traits>

namespace detail
{

template <std::size_t sz, typename A, typename ...B>
struct max_align
{
  static constexpr auto const align = max_align<
    (alignof(A) > sz) ? alignof(A) : sz,
    B...
  >::align;
};

template <std::size_t sz, typename A>
struct max_align<sz, A>
{
  static constexpr auto const align = (alignof(A) > sz) ? alignof(A) : sz;
};

template <typename T, typename A, typename ...B>
struct max_type
{
  typedef typename max_type<
    typename std::conditional<(sizeof(A) > sizeof(T)), A, T>::type,
    B...
  >::type type;
};

template <typename T, typename A>
struct max_type<T, A>
{
  typedef typename std::conditional<(sizeof(A) > sizeof(T)), A, T>::type type;
};

}

template <typename ...T>
struct variant
{
  template <typename A, typename... B>
  struct max_align_
  {
    static constexpr std::size_t align =
      ::detail::max_align<alignof(A), A, B...>::align;
  };

  static constexpr auto const max_align = max_align_<T...>::align;

  template <typename A, typename... B>
  struct max_type_
  {
    typedef typename ::detail::max_type<A, A, B...>::type type;
  };

  typedef typename max_type_<T...>::type max_type;

private:
  alignas(max_align) char store_[sizeof(max_type)];
};

#endif // VARIANT_HPP
