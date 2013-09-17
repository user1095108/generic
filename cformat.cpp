#include "cformat.hpp"

namespace generic
{

//////////////////////////////////////////////////////////////////////////////
::std::string cformat(char const* format, ...)
{
  va_list ap;

  va_start(ap, format);

  ::std::string::size_type const len(::std::vsnprintf(0, 0, format, ap) + 1);

#ifdef _MSC_VER
  #include "alloca.h"
  char* const s(static_cast<char*>(_malloca(len)));
#else
  char s[len];
#endif

  va_end(ap);

  va_start(ap, format);

  if (::std::vsprintf(s, format, ap) < 0)
  {
    va_end(ap);

    throw cformat_error();
  }
  else
  {
    va_end(ap);

    return {s, len};
  }
}

}
