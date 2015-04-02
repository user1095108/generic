#ifndef GENERIC_CONTAINS_HPP
# define GENERIC_CONTAINS_HPP
# pragma once

namespace generic
{

// contains
//////////////////////////////////////////////////////////////////////////////
template <class Container>
inline bool contains(Container const& c,
  typename Container::key_type const& key) noexcept
{
  return c.end() != c.find(key);
}

}

#endif // GENERIC_CONTAINS_HPP
