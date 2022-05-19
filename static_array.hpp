#ifndef STATIC_ARRAY_HPP
# define STATIC_ARRAY_HPP
# pragma once

#include <algorithm> // std::move()
#include <iterator> // std::begin(), std::end()

namespace gnr
{

template <typename A>
class static_array
{
  A* p_;
  std::size_t const N_;

  template <auto, std::size_t N>
  static constinit inline A storage_[N];

  static constinit inline std::size_t c_; // instance counter

public:
  template <std::size_t N>
  static_array(A (&&a)[N]):
    N_(N)
  {
    [&]<auto ...I>(auto const c, std::index_sequence<I...>)
    {
      (
        (
          I == c ? p_ = storage_<I, N> : nullptr
        ),
        ...
      );
    }(c_++, std::make_index_sequence<128>());

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
