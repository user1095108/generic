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
  std::conditional_t<
    std::is_array_v<A>,
    std::decay_t<A>,
    A*
  > p_;

  template <auto>
  static constinit inline std::aligned_storage<sizeof(A)> storage_;

public:
  static_new(auto&& ...a)
    noexcept(std::is_nothrow_constructible_v<A, decltype(a)...>)
  {
    static constinit std::size_t c; // instance counter

    void* p;

    [&]<auto ...I>(auto const c, std::index_sequence<I...>)
    {
      (
        (
          I == c ? p = &storage_<I> : nullptr
        ),
        ...
      );
    }(c++, std::make_index_sequence<N>());

    p_ = ::new (p) A(std::forward<decltype(a)>(a)...);
  }

  static_new(static_new const&) = delete;
  static_new(static_new&&) = delete;

  //
  static_new& operator=(static_new const&) = delete;
  static_new& operator=(static_new&&) = delete;

  //
  operator decltype(p_)() noexcept { return p_; }
};

}

#endif // STATIC_NEW_HPP
