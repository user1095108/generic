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
class rev
{
  T ref_;

public:
  explicit rev(T&& r) noexcept(noexcept(T(std::forward<T>(r)))) :
    ref_(std::forward<T>(r))
  {
  };

  auto begin() noexcept(noexcept(std::rbegin(ref_)))
  {
    return std::rbegin(ref_);
  }

  auto end() noexcept(noexcept(std::rend(ref_)))
  {
    return std::rend(ref_);
  }

  auto cbegin() noexcept(noexcept(std::crbegin(ref_)))
  {
    return std::crbegin(ref_);
  }

  auto cend() noexcept(noexcept(std::crend(ref_)))
  {
    return std::crend(ref_);
  }
};

}

template <typename T>
auto rev(T&& r) noexcept(noexcept(detail::rev<T>(std::forward<T>(r))))
{
  return detail::rev<T>(std::forward<T>(r));
}

}

#endif // GENERIC_REV_HPP