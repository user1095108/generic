#include "boost/pfr.hpp"

#include <iterator>
#include <memory> //std::addressof()

namespace gnr
{

template <typename S>
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

  static constexpr auto&& get(S& s, std::size_t const i) noexcept
  {
    return [&]<auto ...I>(std::index_sequence<I...>) noexcept -> auto&&
      {
        pointer r{};

        (
          (
            r = I == i ? &boost::pfr::get<I>(s) : r
          ),
          ...
        );

        return *r;
      }(std::make_index_sequence<boost::pfr::tuple_size<S>{}>());
  }

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
  constexpr bool operator==(struct_iterator const other) const noexcept
  {
    return (std::addressof(other.s_) == std::addressof(s_)) &&
      (other.i_ == i_);
  }

  constexpr bool operator!=(struct_iterator const other) const noexcept
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
    return get(s_, i_);
  }

  constexpr auto& operator[](std::size_t const i) const noexcept
  {
    return *(*this + i);
  }
};

template <typename S>
class struct_range
{
  S& s_;

public:
  constexpr explicit struct_range(S& s) noexcept : s_{s}
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

}
