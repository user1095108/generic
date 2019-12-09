#ifndef GENERIC_REV_HPP
# define GENERIC_REV_HPP
# pragma once

#include <iterator>

#include <utility>

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

  template <typename U, std::size_t N>
  explicit rev_impl(U(&&a)[N]) noexcept(
    noexcept(std::move(std::begin(a), std::end(a), std::begin(ref_))))
  {
    std::move(std::begin(a), std::end(a), std::begin(ref_));
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
auto rev(T&& r) noexcept(noexcept(detail::rev_impl<T>(std::forward<T>(r)))) ->
  decltype(std::begin(r),std::end(r),detail::rev_impl<T>(std::forward<T>(r)))
{
  return detail::rev_impl<T>(std::forward<T>(r));
}

template <typename T, std::size_t N>
auto rev(T(&&a)[N]) noexcept(noexcept(detail::rev_impl<T(&&)[N]>(std::move(a))))
{
  return detail::rev_impl<T[N]>(std::move(a));
}

}

#endif // GENERIC_REV_HPP
