#ifndef GNR_STRING_HPP
# define GNR_STRING_HPP
# pragma once

#include <cstring>

#include <algorithm>

#include <iterator>

#include <sstream>

#include <string>

#include <optional>

#include <type_traits>

#include <utility>

#include <vector>

namespace gnr
{

// cstrlen
//////////////////////////////////////////////////////////////////////////////
template <std::size_t N>
inline constexpr std::size_t cstrlen(char const (&)[N]) noexcept
{
  return N - 1;
}

//////////////////////////////////////////////////////////////////////////////
inline constexpr std::size_t cstrlen(char const* const p) noexcept
{
  return *p ? 1 + cstrlen(p + 1) : 0;
}

// stoi
//////////////////////////////////////////////////////////////////////////////
template <typename T,
  typename S,
  typename = std::enable_if_t<
    std::is_same_v<char, std::decay_t<decltype(std::declval<S>()[0])>>
  >
>
inline auto stoi(S const& s) noexcept ->
  decltype(std::begin(s), std::end(s), std::size(s), std::optional<T>())
{
  T r{};

  auto i(std::begin(s)), end(std::end(s));

  if (i == end)
  {
    return {};
  }
  else
  {
    bool positive;

    switch (*i)
    {
      case '+':
        positive = true;
        i = std::next(i);
        break;

      case '-':
        positive = false;
        i = std::next(i);
        break;

      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        positive = true;
        break;

      default:
        return {};
    }

    for (; i != end; i = std::next(i))
    {
      switch (*i)
      {
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
          r = 10 * r + *i - '0';
          break;

        default:
          return {};
      }
    }

    return positive ? r : -r;
  }
}

template <typename T>
inline std::optional<T> stoi(char const* s) noexcept
{
  T r{};

  bool positive;

  switch (*s)
  {
    case '+':
      positive = true;
      ++s;
      break;

    case '-':
      positive = false;
      ++s;
      break;

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
      positive = true;
      break;

    default:
      return {};
  }

  for (; *s; ++s)
  {
    switch (*s)
    {
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        r = 10 * r + *s - '0';
        break;

      default:
        return {};
    }
  }

  return positive ? r : -r;
}

// join
//////////////////////////////////////////////////////////////////////////////
template <typename C>
inline typename C::value_type join(C const& container,
  typename C::value_type const sep) noexcept
{
  if (container.size())
  {
    typename C::value_type r(*container.begin());

    auto const end(container.end());

    for (auto i(std::next(container.begin())); i != end; ++i)
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
  typename C::value_type::value_type const sep) noexcept
{
  if (container.size())
  {
    typename C::value_type r(container.front());

    auto const end(container.end());

    for (typename C::const_iterator i(container.begin() + 1); i != end; ++i)
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
inline std::vector<std::basic_string<CharT, Traits, Allocator> >
split(std::basic_string<CharT, Traits, Allocator> const& s,
  CharT const delim) noexcept
{
  std::stringstream ss(s);

  std::string item;

  std::vector<typename std::decay<decltype(s)>::type> r;

  while (std::getline(ss, item, delim))
  {
    r.push_back(item);
  }

  return r;
}

//////////////////////////////////////////////////////////////////////////////
inline std::vector<std::string>
split(std::string const& s,
  char const* const delims = "\f\n\r\t\v") noexcept
{
  std::vector<typename std::decay<decltype(s)>::type> r;

  auto const S(s.size());
  decltype(s.size()) i{};

  while (i < S)
  {
    while ((i < S) && std::strchr(delims, s[i]))
    {
      ++i;
    }

    if (i == S)
    {
      break;
    }
    // else do nothing

    auto j(i + 1);

    while ((j < S) && !std::strchr(delims, s[j]))
    {
      ++j;
    }

    r.push_back(s.substr(i, j - i));

    i = j + 1;
  }

  return r;
}

// trim
//////////////////////////////////////////////////////////////////////////////
template<class CharT, class Traits, class Allocator>
inline std::basic_string<CharT, Traits, Allocator>&
ltrim(std::basic_string<CharT, Traits, Allocator>& s,
  CharT const* cs = " ") noexcept
{
  s.erase(s.begin(),
    std::find_if(s.begin(), s.end(),
      [cs](char const c) noexcept {
        return !std::strrchr(cs, c);
      }
    )
  );

  return s;
}

template<class CharT, class Traits, class Allocator>
inline std::basic_string<CharT, Traits, Allocator>&
rtrim(std::basic_string<CharT, Traits, Allocator>& s,
  CharT const* cs = " ") noexcept
{
  s.erase(std::find_if(s.rbegin(), s.rend(),
      [cs](char const c) noexcept {
        return !std::strrchr(cs, c);
      }
    ).base(),
    s.end()
  );

  return s;
}

template<class CharT, class Traits, class Allocator>
inline std::basic_string<CharT, Traits, Allocator>&
trim(std::basic_string<CharT, Traits, Allocator>& s,
  CharT const* cs = " ") noexcept
{
  return ltrim(rtrim(s, cs), cs);
}

}

#endif // GNR_STRING_HPP
