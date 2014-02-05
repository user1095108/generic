#ifndef UTILITY_HPP
# define UTILITY_HPP

#include <cstddef>

#include <type_traits>

namespace generic
{

// as_const
template<typename T> constexpr inline T const& as_const(T& t) { return t; }

// contains
template <class Container, class Key>
inline bool contains(Container const& c, Key const& key)
{
  return c.end() != c.find(key);
}

// indices
template <::std::size_t...> struct indices { };

namespace detail
{

template<class, class> struct catenate_indices;

template <::std::size_t ...Is, ::std::size_t ...Js>
struct catenate_indices<indices<Is...>, indices<Js...> >
{
  using indices_type = indices<Is..., Js...>;
};

template <::std::size_t, ::std::size_t, typename = void>
struct expand_indices;

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
struct make_indices : detail::expand_indices<0, A - 1>::indices_type
{
};

template <>
struct make_indices<0> : indices<>
{
};

template <::std::size_t A, ::std::size_t B>
struct make_indices_range : detail::expand_indices<A, B - 1>::indices_type
{
};

template <::std::size_t A>
struct make_indices_range<A, A> : indices<>
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

template <class A, class ...B>
struct all_of : ::std::integral_constant<bool, A{} && all_of<B...>{}>
{
};

template <class A>
struct all_of<A> : ::std::integral_constant<bool, A{}>
{
};

}

#endif // UTILITY_HPP
