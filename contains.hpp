#ifndef GENERIC_CONTAINS_HPP
# define GENERIC_CONTAINS_HPP
# pragma once

#include <type_traits>

namespace generic
{

// contains
//////////////////////////////////////////////////////////////////////////////
template <class Container>
inline ::std::enable_if_t<
  !::std::is_member_function_pointer<&Container::find>,
  bool
>
contains(Container const& c,
  typename Container::key_type const& key) noexcept(
    noexcept(::std::find(c.begin(), c.end, key))
)
{
  auto const end(c.end());

  return end != ::std::find(c.begin(), end, key);
}

//////////////////////////////////////////////////////////////////////////////
template <class Container>
inline ::std::enable_if_t<
  ::std::is_member_function_pointer<&Container::find>,
  bool
>
contains(Container const& c,
  typename Container::key_type const& key) noexcept
{
  return c.end() != c.find(key);
}

}

#endif // GENERIC_CONTAINS_HPP
