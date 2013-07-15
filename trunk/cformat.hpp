#include <cerrno>

#include <cstdarg>

#include <cstdio>

#include <cstring>

#include <ctime>

#include <stdexcept>

#include <string>

namespace generic
{

//////////////////////////////////////////////////////////////////////////////
struct cformat_error : std::runtime_error
{
  cformat_error() : std::runtime_error(std::string("formatting error: ") +
    std::strerror(errno))
  {
  }
};

typedef cformat_error wcformat_error;

std::string cformat(char const*, ...);

//////////////////////////////////////////////////////////////////////////////
template <std::size_t buffer_size>
inline std::wstring wcformat(wchar_t const* format, ...)
{
  static_assert(buffer_size > 0, "buffer_size must be greater than 0");

  wchar_t s[buffer_size];

  va_list ap;

  va_start(ap, format);

  if (std::vswprintf(s, buffer_size, format, ap) >= 0)
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
struct strftime_array_undefined : std::runtime_error
{
  strftime_array_undefined() :
    std::runtime_error("std::strftime() or std::wcsftime() returned zero")
  {
  }
};

typedef strftime_array_undefined wcsftime_array_undefined;

//////////////////////////////////////////////////////////////////////////////
template <std::size_t buffer_size>
inline std::string cstrftime(char const* format, struct std::tm const* time)
{
  static_assert(buffer_size > 0, "buffer_size must be greater than 0");

  char s[buffer_size];

  if (!std::strftime(s, buffer_size, format, time))
  {
    throw strftime_array_undefined();
  }
  // else do nothing

  return s;
}

//////////////////////////////////////////////////////////////////////////////
template <std::size_t buffer_size>
inline std::wstring wcstrftime(wchar_t const* format,
  struct std::tm const* time)
{
//static_assert(buffer_size > 0, "buffer_size must be greater than 0");

  wchar_t s[buffer_size];

  if (!std::wcsftime(s, buffer_size, format, time))
  {
    throw wcsftime_array_undefined();
  }
  // else do nothing

  return s;
}

}
