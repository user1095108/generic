#ifndef GNR_ANYFIND_HPP
# define GNR_ANYFIND_HPP
# pragma once

#include <algorithm>

#include <iterator>

#include <optional>

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
static constexpr typename std::remove_all_extents_t<T>*
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
static constexpr typename std::remove_all_extents_t<T>*
end(T (&array)[N]) noexcept
{
  return end(array[N - 1]);
}

template <class Container, class Key>
inline auto find(Container& c, Key const& k, int)
  noexcept(noexcept(c.find(k))) -> std::optional<decltype(c.find(k))>
{
  auto const i(c.find(k));

  return c.end() == i ?
    std::optional<decltype(c.find(k))>() :
    i;
}

template <class Container, class Key>
inline auto find(Container& c, Key const& k, char,
  std::enable_if_t<!std::is_array<Container>{}>* = {})
  noexcept(noexcept(std::find(std::begin(c), std::end(c), k))) ->
  std::optional<decltype(std::find(std::begin(c), std::end(c), k))>
{
  auto const e(std::end(c));

  auto const i(std::find(std::begin(c), e, k));

  return e == i ?
    std::optional<decltype(std::find(std::begin(c), e, k))>() :
    i;
}

template <class Container, class Key>
inline auto find(Container& c, Key const& k, char,
  std::enable_if_t<std::is_array<Container>{}>* = {})
  noexcept(
    noexcept(std::find(::gnr::detail::begin(c), ::gnr::detail::end(c), k))
  ) -> std::optional<decltype(
    std::find(::gnr::detail::begin(c), ::gnr::detail::end(c), k))>
{
  auto const e(::gnr::detail::end(c));

  auto const i(std::find(::gnr::detail::begin(c), e, k));

  return e == i ?
    std::optional<decltype(std::find(::gnr::detail::begin(c), e, k))>() :
    i;
}

}

template <class Container, class Key>
inline auto anyfind(Container& c, Key const& k) noexcept(
  noexcept(detail::find(c, k, 0))
)
{
  return detail::find(c, k, 0);
}

template <class Container, class Key, typename F>
inline void anyfind(Container& c, Key const& k, F&& f) noexcept(
  noexcept(f(detail::find(c, k, 0)))
)
{
  f(detail::find(c, k, 0));
}

}

#endif // GNR_ANYFIND_HPP
