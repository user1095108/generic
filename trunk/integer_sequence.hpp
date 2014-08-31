#ifndef GENERIC_INTEGER_SEQUENCE_HPP
# define GENERIC_INTEGER_SEQUENCE_HPP
# pragma once

#include <cstddef>

namespace generic
{

template <typename T, T ...Is>
struct integer_sequence
{
  static_assert(::std::is_integral<T>{} &&
    !::std::is_same<typename ::std::decay<T>::type, bool>{},
    "T must be integral and not bool");

  static constexpr ::std::size_t size() noexcept { return sizeof...(Is); }
};

template <::std::size_t ...Is>
using index_sequence = integer_sequence<::std::size_t, Is...>;

namespace detail
{

template<typename, typename, typename> struct catenate_indices;

template <typename T, T ...Is, T ...Js>
struct catenate_indices<T, integer_sequence<T, Is...>,
  integer_sequence<T, Js...> >
{
  using type = integer_sequence<T, Is..., Js...>;
};

template <typename T, T, T, typename = void>
struct expand_indices;

template <typename T, T A, T B>
struct expand_indices<T, A, B, typename ::std::enable_if<A != B>::type>
{
  static_assert(A < B, "A > B");
  using type = typename catenate_indices<T,
    typename expand_indices<T, A, (A + B) / 2>::type,
    typename expand_indices<T, (A + B) / 2 + 1, B>::type
  >::type;
};

template <typename T, T A, T B>
struct expand_indices<T, A, B, typename ::std::enable_if<A == B>::type>
{
  using type = integer_sequence<T, A>;
};

}

template <std::size_t ...Is>
using index_sequence = integer_sequence<::std::size_t, Is...>;

template <typename T, T A, T B>
struct make_integer_range : detail::expand_indices<T, A, B - 1>::type
{
};

template <typename T, T A>
struct make_integer_range<T, A, A> : integer_sequence<T>
{
};

template <::std::size_t A, ::std::size_t B>
using make_index_range = make_integer_range<decltype(A), A, B>;

template <typename T, T N>
using make_integer_sequence = make_integer_range<T, 0, N>;

template <::std::size_t N>
using make_index_sequence = make_integer_sequence<decltype(N), N>;

template <typename ...T>
using index_sequence_for = make_index_sequence<sizeof...(T)>;

}

#endif // GENERIC_INTEGER_SEQUENCE_HPP
