#ifndef GENERIC_REV_HPP
# define GENERIC_REV_HPP
# pragma once

#include <iterator>

#include <type_traits>

namespace gnr
{

namespace detail
{

template <typename T>
class rev_impl
{
  T ref_;

public:
  explicit rev_impl(T&& r) noexcept :
    ref_(std::forward<T>(r))
  {
  };

  auto begin() const noexcept(noexcept(std::rbegin(ref_)))
  {
    return std::rbegin(std::forward<T>(ref_));
  }

  auto end() const noexcept(noexcept(std::rend(ref_)))
  {
    return std::rend(std::forward<T>(ref_));
  }

  auto cbegin() const noexcept(noexcept(std::crbegin(ref_)))
  {
    return std::crbegin(std::forward<T>(ref_));
  }

  auto cend() const noexcept(noexcept(std::crend(ref_)))
  {
    return std::crend(std::forward<T>(ref_));
  }
};

}

template <typename T, typename = std::enable_if_t<std::is_reference<T>{}>>
auto rev(T&& ref) noexcept(noexcept(
  detail::rev_impl<T&&>(std::forward<T>(ref))))
{
  return detail::rev_impl<T&&>(std::forward<T>(ref));
}

}

#endif // GENERIC_REV_HPP
