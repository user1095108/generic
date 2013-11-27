#include <cerrno>

#include <cstdarg>

#include <cstdio>

#include <cstring>

#include <ctime>

#include <stdexcept>

#if defined(_MSC_VER)
# include <malloc.h>
#endif

#include "stackallocator.hpp"

//////////////////////////////////////////////////////////////////////////////
struct cformat_error : ::std::runtime_error
{
  cformat_error() : ::std::runtime_error(::std::string("formatting error: ") +
    ::std::strerror(errno))
  {
  }
};

typedef cformat_error wcformat_error;

//////////////////////////////////////////////////////////////////////////////
inline ::std::string cformat(char const* format, ...)
{
  va_list ap;

  int len;

  {
    va_start(ap, format);

#if !defined(_MSC_VER)
    char tmp[64];
    len = ::std::vsnprintf(tmp, sizeof(tmp), format, ap);
#else
    len = ::std::vsnprintf(static_cast<char*>(_alloca(64)), sizeof(tmp),
      format, ap);
#endif

    va_end(ap);

    if (len < 0)
    {
      throw cformat_error();
    }
    else if (len <= decltype(len)(sizeof(tmp)))
    {
      return {tmp, ::std::string::size_type(len)};
    }
  }

  va_start(ap, format);

#if !defined(_MSC_VER)
  char s[len];
  ::std::vsnprintf(s, len, format, ap);
#else
  ::std::vsnprintf(static_cast<char*>(_alloca(len)), len, format, ap);
#endif

  va_end(ap);

  return {s, ::std::string::size_type(len)};
}

//////////////////////////////////////////////////////////////////////////////
template <typename S>
inline void cformat(S& r, char const* format, ...)
{
  va_list ap;

  int len;

  {
    va_start(ap, format);

#if !defined(_MSC_VER)
    char tmp[64];
    len = ::std::vsnprintf(tmp, sizeof(tmp), format, ap);
#else
    len = ::std::vsnprintf(static_cast<char*>(_alloca(64)), sizeof(tmp),
      format, ap);
#endif

    va_end(ap);

    if (len < 0)
    {
      throw cformat_error();
    }
    else if (len <= decltype(len)(sizeof(tmp)))
    {
      r.assign(tmp, len);

      return;
    }
  }

  va_start(ap, format);

#if !defined(_MSC_VER)
  char s[len];
  ::std::vsnprintf(s, len, format, ap);
#else
  ::std::vsnprintf(static_cast<char*>(_alloca(len)), len, format, ap);
#endif

  va_end(ap);

  r.assign(s, len);
}

//////////////////////////////////////////////////////////////////////////////
template <::std::size_t buffer_size = 128>
inline ::std::wstring wcformat(wchar_t const* format, ...)
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

    throw wcformat_error();
  }
}

//////////////////////////////////////////////////////////////////////////////
struct strftime_array_undefined : ::std::runtime_error
{
  strftime_array_undefined() :
    ::std::runtime_error("::std::strftime() or ::std::wcsftime() returned zero")
  {
  }
};

typedef strftime_array_undefined wcsftime_array_undefined;

//////////////////////////////////////////////////////////////////////////////
template <::std::size_t buffer_size = 128>
inline ::std::string cstrftime(char const* format,
  struct ::std::tm const* time)
{
  static_assert(buffer_size > 0, "buffer_size must be greater than 0");

  char s[buffer_size];

  if (!::std::strftime(s, buffer_size, format, time))
  {
    throw strftime_array_undefined();
  }
  // else do nothing

  return s;
}

//////////////////////////////////////////////////////////////////////////////
template <::std::size_t buffer_size = 128>
inline ::std::wstring wcstrftime(wchar_t const* format,
  struct ::std::tm const* time)
{
//static_assert(buffer_size > 0, "buffer_size must be greater than 0");

  wchar_t s[buffer_size];

  if (!::std::wcsftime(s, buffer_size, format, time))
  {
    throw wcsftime_array_undefined();
  }
  // else do nothing

  return s;
}
