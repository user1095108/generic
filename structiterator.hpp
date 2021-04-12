#ifndef GNR_STRUCTITERATOR_HPP
# define GNR_STRUCTITERATOR_HPP
# pragma once

#include <iterator>

#include <memory> //std::addressof()

#include "boost/pfr.hpp"

namespace gnr
{

namespace detail::struct_iterator
{

template <class S>
constexpr auto all_same() noexcept
{
  if constexpr (constexpr auto N(boost::pfr::tuple_size_v<S>); N > 1)
  {
    return [&]<auto ...I>(std::index_sequence<I...>) noexcept
      {
        return (
            (
              std::is_same_v<
                boost::pfr::tuple_element_t<I, S>,
                boost::pfr::tuple_element_t<I + 1, S>
              >
            ) && ...
          );
      }(std::make_index_sequence<N - 1>());
  }
  else
  {
    return true;
  }
}

template <class S>
static constexpr auto is_proper_v(std::is_class_v<S> && all_same<S>());

}

template <typename S>
  requires detail::struct_iterator::is_proper_v<S>
class struct_iterator
{
  S& s_;
  std::size_t i_;

public:
  using iterator_category = std::random_access_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using pointer = decltype(&boost::pfr::get<0>(s_));
  using reference = decltype(boost::pfr::get<0>(s_));
  using value_type = std::remove_reference_t<reference>;

public:
  constexpr explicit struct_iterator(S& s,
    std::size_t const i = boost::pfr::tuple_size<S>{}) noexcept:
    s_{s},
    i_{i}
  {
  }

  // increment, decrement
  constexpr auto& operator++() noexcept { return ++i_, *this; }
  constexpr auto& operator--() noexcept { return --i_, *this; }

  constexpr auto operator++(int) const noexcept
  {
    return struct_iterator(s_, i_ + 1);
  }

  constexpr auto operator--(int) const noexcept
  {
    return struct_iterator(s_, i_ - 1);
  }

  // arithmetic
  constexpr auto operator-(struct_iterator const other) const noexcept
  {
    return difference_type(i_ - other.i_);
  }

  constexpr auto operator+(std::size_t const N) const noexcept
  {
    return struct_iterator(s_, i_ + N);
  }

  constexpr auto operator-(std::size_t const N) const noexcept
  {
    return struct_iterator(s_, i_ - N);
  }

  // comparison
  constexpr auto operator==(struct_iterator const other) const noexcept
  {
    return (std::addressof(other.s_) == std::addressof(s_)) &&
      (other.i_ == i_);
  }

  constexpr auto operator!=(struct_iterator const other) const noexcept
  {
    return !(*this == other);
  }

  constexpr auto operator<(struct_iterator const other) const noexcept
  {
    return i_ < other.i_;
  }

  //
  constexpr auto& operator*() const noexcept
  {
    return [&]<auto ...I>(std::index_sequence<I...>) noexcept -> auto&
      {
        return *pointer(
          (
            (
              I == i_ ?
                std::uintptr_t(&boost::pfr::get<I>(s_)) :
                std::uintptr_t{}
            ) |
            ...
          )
        );
      }(std::make_index_sequence<boost::pfr::tuple_size_v<S>>());
  }

  constexpr auto& operator[](std::size_t const i) const noexcept
  {
    return *(*this + i);
  }
};

template <typename S>
  requires detail::struct_iterator::is_proper_v<S>
class range
{
  S& s_;

public:
  constexpr explicit range(S& s) noexcept : s_{s}
  {
  }

  constexpr auto begin() const noexcept
  {
    return struct_iterator{s_, {}};
  }

  constexpr auto end() const noexcept
  {
    return struct_iterator{s_};
  }
};

template <typename S>
  requires detail::struct_iterator::is_proper_v<S>
constexpr auto begin(S& s) noexcept
{
  return struct_iterator{s, {}};
}

template <typename S>
  requires detail::struct_iterator::is_proper_v<S>
constexpr auto end(S& s) noexcept
{
  return struct_iterator{s};
}

template <typename S>
  requires detail::struct_iterator::is_proper_v<S>
constexpr auto size(S& s) noexcept
{
  return boost::pfr::tuple_size_v<S>;
}

}

#endif // GNR_STRUCTITERATOR_HPP
