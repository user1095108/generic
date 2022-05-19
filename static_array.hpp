#ifndef STATIC_ARRAY_HPP
# define STATIC_ARRAY_HPP
# pragma once

#include <algorithm> // std::move()
#include <iterator> // std::begin(), std::end()

#include "static_new.hpp"

namespace gnr
{

template <typename A>
class static_array
{
  A* p_;
  std::size_t const N_;

public:
  template <std::size_t N>
  static_array(A (&&a)[N])
    noexcept(std::is_nothrow_move_assignable_v<A>):
    p_(gnr::static_new<A[N]>()),
    N_(N)
  {
    std::move(std::begin(a), std::end(a), begin());
  }

  static_array(static_array const&) = default;
  static_array(static_array&&) = default;

  //
  static_array& operator=(static_array const&) = delete;
  static_array& operator=(static_array&&) = delete;

  //
  auto begin() const noexcept { return p_; }
  auto end() const noexcept { return p_ + N_; }
  auto size() const noexcept { return N_; }
};

}

#endif // STATIC_ARRAY_HPP
