#ifndef GENERIC_REV_HPP
# define GENERIC_REV_HPP
# pragma once

#include <iterator>

#include <type_traits>

namespace gnr
{

template <typename T>
class rev
{
  T ref_;

public:
  explicit rev(T& r) noexcept(noexcept(T(std::forward<T>(r)))) :
    ref_(std::forward<T>(r))
  {
  };

  explicit rev(T&& r) noexcept(noexcept(T(std::forward<T>(r)))) :
    ref_(std::forward<T>(r))
  {
  };

  auto begin() const noexcept(noexcept(std::rbegin(ref_)))
  {
    return std::rbegin(ref_);
  }

  auto end() const noexcept(noexcept(std::rend(ref_)))
  {
    return std::rend(ref_);
  }

  auto cbegin() const noexcept(noexcept(std::crbegin(ref_)))
  {
    return std::crbegin(ref_);
  }

  auto cend() const noexcept(noexcept(std::crend(ref_)))
  {
    return std::crend(ref_);
  }
};

}

#endif // GENERIC_REV_HPP
