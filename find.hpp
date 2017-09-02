#ifndef GNR_FIND_HPP
# define GNR_FIND_HPP
# pragma once

#include <algorithm>

#include <type_traits>

namespace gnr
{

namespace
{

template <typename C, typename = std::size_t>
struct has_find: std::false_type
{
};

template <typename C>
struct has_find<C,
  decltype(
    sizeof(
      static_cast<typename C::iterator(C::*)(
        typename C::key_type const&)>(&C::find)
    ) |
    sizeof(
      static_cast<typename C::const_iterator(C::*)(
        typename C::key_type const&) const>(&C::find)
    )
  )
> : std::true_type
{
};

}

template <class Container, class Key, typename F>
inline std::enable_if_t<!has_find<Container>{}>
find_first(Container& c, Key const& k, F&& f) noexcept(
  noexcept(f(std::declval<typename Container::const_iterator>()))
)
{
  f(std::find(c.begin(), c.end(), k));
}

template <class Container, class Key, typename F>
inline std::enable_if_t<has_find<Container>{}>
find_first(Container& c, Key const& k, F&& f) noexcept(
  noexcept(f(std::declval<typename Container::const_iterator>()))
)
{
  f(c.find(k));
}

}

#endif // GNR_FIND_HPP
