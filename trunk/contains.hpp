#ifndef GENERIC_CONTAINS_HPP
# define GENERIC_CONTAINS_HPP
# pragma once

namespace generic
{

// contains
//////////////////////////////////////////////////////////////////////////////
template <class Container, class Key>
inline bool contains(Container const& c, Key const& key) noexcept
{
  return c.end() != c.find(key);
}

}

#endif // GENERIC_CONTAINS_HPP
