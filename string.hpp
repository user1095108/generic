#ifndef STRING_HPP
# define STRING_HPP

#include <cstring>

#include <string>

namespace generic
{

// trimming
//////////////////////////////////////////////////////////////////////////////
inline ::std::string& ltrim(::std::string& s)
{
  s.erase(s.begin(), ::std::find_if(s.begin(), s.end(),
    [](char const c){ return !::std::isspace(c); }));

  return s;
}

inline ::std::string& rtrim(::std::string& s)
{
  s.erase(s.begin(), ::std::find_if(s.begin(), s.end(),
    [](char const c){ return !::std::isspace(c); }));

  return s;
}

inline ::std::string& trim(::std::string& s)
{
  return ltrim(rtrim(s));
}

}

#endif // STRING_HPP
