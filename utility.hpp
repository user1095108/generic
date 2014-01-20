#ifndef UTILITY_HPP
# define UTILITY_HPP

#include <cstddef>

#include <type_traits>

namespace generic
{

template <::std::size_t...> struct indices { };

namespace detail
{

// indices
template<class A, class B> struct catenate_indices;

template <::std::size_t ...Is, ::std::size_t ...Js>
struct catenate_indices<indices<Is...>, indices<Js...> >
{
  using indices_type = indices<Is..., Js...>;
};

template <::std::size_t, ::std::size_t, typename = void> struct expand_indices;

template <::std::size_t A, ::std::size_t B>
struct expand_indices<A, B, typename ::std::enable_if<A == B>::type>
{
  using indices_type = indices<A>;
};

template <::std::size_t A, ::std::size_t B>
struct expand_indices<A, B, typename ::std::enable_if<A != B>::type>
{
  static_assert(A < B, "A > B");
  using indices_type = typename catenate_indices<
    typename expand_indices<A, (A + B) / 2>::indices_type,
    typename expand_indices<(A + B) / 2 + 1, B>::indices_type
  >::indices_type;
};

}

template <::std::size_t A>
struct make_indices : detail::expand_indices<0, A>::indices_type
{
};

template <::std::size_t A, ::std::size_t B>
struct make_indices_range : detail::expand_indices<A, B>::indices_type
{
};

// sequences
template <::std::size_t I, typename A, typename ...B>
struct type_at : type_at<I - 1, B...>
{
};

template <typename A, typename ...B>
struct type_at<0, A, B...>
{
  using type = A;
};

template <typename A, typename ...B>
struct front
{
  using type = A;
};

template <typename A, typename ...B>
struct back : back<B...>
{
};

template <typename A>
struct back<A>
{
  using type = A;
};

template <bool B>
using bool_ = ::std::integral_constant<bool, B>;

template <class A, class ...B>
struct all_of : bool_<A::value && all_of<B...>::value> { };

template <class A>
struct all_of<A> : bool_<A::value> { };

}

#endif // UTILITY_HPP
