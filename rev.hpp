#ifndef GNR_REV_HPP
# define GNR_REV_HPP
# pragma once

#include <initializer_list>
#include <iterator>
#include <type_traits>

namespace gnr
{

template <typename T>
struct rev
{
  static_assert(std::is_reference_v<T>);
  T r_;

  //
  auto begin() noexcept(noexcept(std::rbegin(r_))) { return std::rbegin(r_); }
  auto end() noexcept(noexcept(std::rend(r_))) { return std::rend(r_); }
};

template <typename T>
rev(std::initializer_list<T>) -> rev<std::initializer_list<T>&&>;

template <typename T>
rev(T&&) -> rev<T&&>;

}

#endif // GNR_REV_HPP
