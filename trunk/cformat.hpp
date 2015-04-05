#ifndef CFORMAT_HPP
# define CFORMAT_HPP
# pragma once

#include <cerrno>

#include <cstdarg>

#include <cstdio>

#include <cstring>

#include <ctime>

#include <stdexcept>

#include <string>

#if defined(_MSC_VER)
# include <malloc.h>
#endif // _MSC_VER

namespace generic
{

//////////////////////////////////////////////////////////////////////////////
inline ::std::string cformat(char const* const format, ...)
{
  va_list ap;

  int len;

  {
    va_start(ap, format);

    char tmp[64];
    len = ::std::vsnprintf(tmp, sizeof(tmp), format, ap);

    va_end(ap);

    if (len < 0)
    {
      return {};
    }
    else if (len <= decltype(len)(sizeof(tmp)))
    {
      return {tmp, ::std::string::size_type(len)};
    }
    // else do nothing
  }

  va_start(ap, format);

#if defined(_MSC_VER)
  auto const s(static_cast<char*>(_alloca(len + 1)));
#else
  char s[len + 1];
#endif // _MSC_VER

  ::std::vsnprintf(s, len + 1, format, ap);

  va_end(ap);

  return {s, ::std::string::size_type(len)};
}

//////////////////////////////////////////////////////////////////////////////
template <typename S>
inline typename ::std::enable_if<!::std::is_const<S>{}>::type
cformat(S& r, char const* const format, ...)
{
  va_list ap;

  int len;

  {
    va_start(ap, format);

    char tmp[64];
    len = ::std::vsnprintf(tmp, sizeof(tmp), format, ap);

    va_end(ap);

    if (len < 0)
    {
      return {};
    }
    else if (len <= decltype(len)(sizeof(tmp)))
    {
      r.assign(tmp, len);

      return;
    }
    // else do nothing
  }

  va_start(ap, format);

#if defined(_MSC_VER)
  auto const s(static_cast<char*>(_alloca(len + 1)));
#else
  char s[len + 1];
#endif // _MSC_VER

  ::std::vsnprintf(s, len + 1, format, ap);

  va_end(ap);

  r.assign(s, len);
}

//////////////////////////////////////////////////////////////////////////////
template <::std::size_t buffer_size = 128>
inline ::std::wstring wcformat(wchar_t const* const format, ...)
{
  static_assert(buffer_size > 0, "buffer_size must be greater than 0");

  wchar_t s[buffer_size];

  va_list ap;

  va_start(ap, format);

  if (::std::vswprintf(s, buffer_size, format, ap) >= 0)
  {
    va_end(ap);

    return s;
  }
  else
  {
    va_end(ap);

    return {};
  }
}

//////////////////////////////////////////////////////////////////////////////
template <::std::size_t buffer_size = 128>
inline ::std::string cstrftime(char const* const format,
  struct ::std::tm const* time)
{
  static_assert(buffer_size > 0, "buffer_size must be greater than 0");

#if defined(_MSC_VER)
  auto const s(static_cast<char*>(_alloca(buffer_size)));
#else
  char s[buffer_size];
#endif // _MSC_VER

  return ::std::strftime(s, buffer_size, format, time) ? s : ::std::string();
}

//////////////////////////////////////////////////////////////////////////////
template <::std::size_t buffer_size = 128>
inline ::std::wstring wcstrftime(wchar_t const* const format,
  struct ::std::tm const* time)
{
  static_assert(buffer_size > 0, "buffer_size must be greater than 0");

#if defined(_MSC_VER)
  auto const s(static_cast<char*>(_alloca(buffer_size)));
#else
  wchar_t s[buffer_size];
#endif // _MSC_VER

  return wcsftime(s, buffer_size, format, time) ? s : ::std::wstring();
}

}

#endif // CFORMAT_HPP
