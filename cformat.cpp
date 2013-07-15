#include <memory>

#include "cformat.hpp"

namespace generic
{

//////////////////////////////////////////////////////////////////////////////
std::string cformat(char const* format, ...)
{
  va_list ap;

  va_start(ap, format);

  std::unique_ptr<char[]> s;

  try
  {
    s.reset(new char[vsnprintf(0, 0, format, ap) + 1]);
  }
  catch (...)
  {
    va_end(ap);

    throw;
  }

  va_end(ap);

  va_start(ap, format);

  if (std::vsprintf(s.get(), format, ap) < 0)
  {
    va_end(ap);

    throw cformat_error();
  }
  else
  {
    va_end(ap);

    return s.get();
  }
}

}
