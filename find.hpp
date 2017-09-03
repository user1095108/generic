#ifndef GNR_FIND_HPP
# define GNR_FIND_HPP
# pragma once

#include <algorithm>

namespace gnr
{

namespace detail
{

template <class Container, class Key>
inline auto find(Container& c, Key const& k, int)
  noexcept(noexcept(c.find(k))) -> decltype(c.find(k))
{
  return c.find(k);
}

template <class Container, class Key>
inline auto find(Container& c, Key const& k, char)
  noexcept(noexcept(std::find(std::begin(c), std::end(c), k))) ->
  decltype(std::find(std::begin(c), std::end(c), k))
{
  return std::find(std::begin(c), std::end(c), k);
}

}

template <class Container, class Key>
inline auto any_find(Container& c, Key const& k) noexcept(
  noexcept(detail::find(c, k, 0))
)
{
  return detail::find(c, k, 0);
}

template <class Container, class Key, typename F>
inline void any_find(Container& c, Key const& k, F&& f) noexcept(
  noexcept(f(detail::find(c, k, 0)))
)
{
  f(detail::find(c, k, 0));
}

}

#endif // GNR_FIND_HPP
