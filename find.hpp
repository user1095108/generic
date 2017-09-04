#ifndef GNR_FIND_HPP
# define GNR_FIND_HPP
# pragma once

#include <algorithm>

#include <type_traits>

namespace gnr
{

namespace detail
{

template <typename T>
static constexpr T* begin(T& value) noexcept
{
  return &value;
}

template <typename T, std::size_t N>
static constexpr typename std::remove_all_extents<T>::type*
begin(T (&array)[N]) noexcept
{
  return begin(*array);
}

template <typename T>
static constexpr T* end(T& value) noexcept
{
  return &value + 1;
}

template <typename T, std::size_t N>
static constexpr typename std::remove_all_extents<T>::type*
end(T (&array)[N]) noexcept
{
  return end(array[N - 1]);
}

template <class Container, class Key>
inline auto find(Container& c, Key const& k, int)
  noexcept(noexcept(c.find(k))) -> decltype(c.find(k))
{
  return c.find(k);
}

template <class Container, class Key>
inline auto find(Container& c, Key const& k, char,
  std::enable_if_t<!std::is_array<Container>{}>* = {})
  noexcept(noexcept(std::find(std::begin(c), std::end(c), k))) ->
  decltype(std::find(std::begin(c), std::end(c), k))
{
  return std::find(std::begin(c), std::end(c), k);
}

template <class Container, class Key>
inline auto find(Container& c, Key const& k, char,
  std::enable_if_t<std::is_array<Container>{}>* = {})
  noexcept(noexcept(std::find(begin(c), end(c), k))) ->
  decltype(std::find(begin(c), end(c), k))
{
  return std::find(begin(c), end(c), k);
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
