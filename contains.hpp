#ifndef GENERIC_CONTAINS_HPP
# define GENERIC_CONTAINS_HPP
# pragma once

#include <algorithm>

#include <type_traits>

namespace generic
{

namespace
{

// contains
//////////////////////////////////////////////////////////////////////////////
template <class Container>
auto contains(Container const& c,
  typename Container::key_type const& key, int) noexcept ->
  decltype(c.find(key), true)
{
  return c.end() != c.find(key);
}

//////////////////////////////////////////////////////////////////////////////
template <class Container>
auto contains(Container const& c,
  typename Container::value_type const& key, long) noexcept(
    noexcept(::std::find(c.begin(), c.end, key))
)
{
  auto const end(c.end());

  return end != ::std::find(c.begin(), end, key);
}

}

//////////////////////////////////////////////////////////////////////////////
template <class Container, typename T>
auto contains(Container const& c, T const& key) noexcept
{
  return contains(c, key, 0);
}

}

#endif // GENERIC_CONTAINS_HPP
