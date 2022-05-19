#ifndef STATIC_NEW_HPP
# define STATIC_NEW_HPP
# pragma once

#include <new>
#include <type_traits>

namespace gnr
{

template <typename A, std::size_t N = 128>
class static_new
{
  A* p_;

  template <auto>
  static constinit inline A storage_;

  static constinit inline std::size_t c_; // instance counter

public:
  static_new() noexcept
  {
    [&]<auto ...I>(auto const c, std::index_sequence<I...>)
    {
      (
        (
          I == c ? p_ = &storage_<I> : nullptr
        ),
        ...
      );
    }(c_++, std::make_index_sequence<N>());
  }

  static_new(static_new const&) = delete;
  static_new(static_new&&) = delete;

  //
  static_new& operator=(static_new const&) = delete;
  static_new& operator=(static_new&&) = delete;

  //
  operator A*() noexcept { return p_; }
};

}

#endif // STATIC_NEW_HPP
