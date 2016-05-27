#ifndef GENERIC_CONTAINS_HPP
# define GENERIC_CONTAINS_HPP
# pragma once

#include <algorithm>

namespace generic
{

namespace
{

// contains
//////////////////////////////////////////////////////////////////////////////
template <class Container>
inline auto contains(Container const& c,
  typename Container::key_type const& key, int) noexcept(
    noexcept(c.end(), c.find(key))) ->
  decltype(c.find(key), true)
{
  return c.end() != c.find(key);
}

//////////////////////////////////////////////////////////////////////////////
template <class Container>
inline auto contains(Container const& c,
  typename Container::value_type const& key, long) noexcept(
    noexcept(c.end(), ::std::find(c.begin(), c.end(), key))
)
{
  auto const cend(c.cend());

  return cend != ::std::find(c.cbegin(), cend, key);
}

}

//////////////////////////////////////////////////////////////////////////////
template <class Container, typename T>
inline auto contains(Container const& c, T const& key) noexcept(
  noexcept(contains(c, key, 0))
)
{
  return contains(c, key, 0);
}

}

#endif // GENERIC_CONTAINS_HPP
