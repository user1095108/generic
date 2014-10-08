#ifndef STRING_HPP
# define STRING_HPP

#include <cstring>

#include <algorithm>

#include <sstream>

#include <string>

#include <vector>

namespace generic
{

// cstrlen
//////////////////////////////////////////////////////////////////////////////
template <::std::size_t N>
inline constexpr ::std::size_t cstrlen(char const (&)[N])
{
  return N - 1;
}

//////////////////////////////////////////////////////////////////////////////
inline constexpr ::std::size_t cstrlen(char const* const p) noexcept
{
  return *p ? 1 + cstrlen(p + 1) : 0;
}

// join
//////////////////////////////////////////////////////////////////////////////
template <typename C>
inline typename C::value_type join(C const& container,
  typename C::value_type const sep)
{
  if (container.size())
  {
    typename C::value_type r(container.front());

    auto const end(container.cend());

    for (typename C::const_iterator i(container.cbegin() + 1); i != end; ++i)
    {
      r += sep + *i;
    }

    return r;
  }
  else
  {
    return typename C::value_type();
  }
}

//////////////////////////////////////////////////////////////////////////////
template <typename C>
inline typename C::value_type join(C const& container,
  typename C::value_type::value_type const sep)
{
  if (container.size())
  {
    typename C::value_type r(container.front());

    auto const end(container.cend());

    for (typename C::const_iterator i(container.cbegin() + 1); i != end; ++i)
    {
      r += sep + *i;
    }

    return r;
  }
  else
  {
    return typename C::value_type();
  }
}

// split
//////////////////////////////////////////////////////////////////////////////
template<class CharT, class Traits, class Allocator>
inline ::std::vector<::std::basic_string<CharT, Traits, Allocator> >
split(::std::basic_string<CharT, Traits, Allocator> const& s,
  CharT const delim)
{
  ::std::stringstream ss(s);

  ::std::string item;

  ::std::vector<typename ::std::decay<decltype(s)>::type> r;

  while (::std::getline(ss, item, delim))
  {
    r.push_back(item);
  }

  return r;
}

// trim
//////////////////////////////////////////////////////////////////////////////
template<class CharT, class Traits, class Allocator>
inline ::std::basic_string<CharT, Traits, Allocator>&
ltrim(::std::basic_string<CharT, Traits, Allocator>& s)
{
  s.erase(s.begin(), ::std::find_if(s.begin(), s.end(),
    [](char const c){ return !::std::isspace(c); }));

  return s;
}

template<class CharT, class Traits, class Allocator>
inline ::std::basic_string<CharT, Traits, Allocator>&
rtrim(::std::basic_string<CharT, Traits, Allocator>& s)
{
  s.erase(::std::find_if(s.rbegin(), s.rend(),
    [](char const c){ return !::std::isspace(c); }).base(), s.end());

  return s;
}

template<class CharT, class Traits, class Allocator>
inline ::std::basic_string<CharT, Traits, Allocator>&
trim(::std::basic_string<CharT, Traits, Allocator>& s)
{
  return ltrim(rtrim(s));
}

}

#endif // STRING_HPP
