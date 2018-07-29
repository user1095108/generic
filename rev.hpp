#ifndef GENERIC_REV_HPP
# define GENERIC_REV_HPP
# pragma once

#include <vector>

#include <iterator>

namespace gnr
{

namespace detail
{

template <typename T>
class rev_impl
{
  T ref_;

public:
  template <typename U>
  explicit rev_impl(U&& r) noexcept(noexcept(T(std::forward<U>(r)))) :
    ref_(std::forward<U>(r))
  {
  }

  auto begin() noexcept(noexcept(std::rbegin(ref_)))
  {
    return std::rbegin(ref_);
  }

  auto end() noexcept(noexcept(std::rend(ref_)))
  {
    return std::rend(ref_);
  }
};

}

template <typename T>
auto rev(T&& r) noexcept(noexcept(detail::rev_impl<T>(std::forward<T>(r))))
{
  return detail::rev_impl<T>(std::forward<T>(r));
}

template <typename T>
auto rev(std::initializer_list<T> r) noexcept(
  noexcept(detail::rev_impl<std::vector<T>>(r)))
{
  return detail::rev_impl<std::vector<T>>(r);
}

}

#endif // GENERIC_REV_HPP
