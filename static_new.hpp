#ifndef STATIC_NEW_HPP
# define STATIC_NEW_HPP
# pragma once

#include <memory>
#include <type_traits>

namespace gnr
{

template <typename A, std::size_t N = 128>
class static_new
{
  std::conditional_t<
    std::is_array_v<A>,
    std::decay_t<A>,
    A*
  > p_;

  static constinit inline std::size_t c_; // instance counter

  template <auto>
  static constinit inline std::aligned_storage_t<
    sizeof(A),
    alignof(A)
  > storage_;

  static constinit inline struct deleter
  {
    ~deleter()
      noexcept(std::is_nothrow_destructible_v<A>)
    {
      [&]<auto ...I>(auto const c, std::index_sequence<I...>)
      {
        (
          (
            I < c ?
              std::destroy_at(
                std::launder(reinterpret_cast<A*>(&storage_<I>))
              ) :
              void()
          ),
          ...
        );
      }(c_, std::make_index_sequence<N>());
    }
  } const d_;

public:
  explicit static_new(auto&& ...a)
    noexcept(std::is_nothrow_constructible_v<A, decltype(a)...>)
  {
    [&]<auto ...I>(auto const c, std::index_sequence<I...>)
    {
      (
        (
          I == c ?
            p_ = ::new (&storage_<I>) A(std::forward<decltype(a)>(a)...) :
            nullptr
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
  operator auto() noexcept { return p_; }
};

}

#endif // STATIC_NEW_HPP
