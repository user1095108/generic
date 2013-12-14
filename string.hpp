#ifndef STRING_HPP
# define STRING_HPP

#include <string>

namespace generic
{

inline ::std::string& trim(::std::string& s)
{
  s.erase(s.begin(), ::std::find_if(s.begin(), s.end(),
    [](char const c){ return !::std::isspace(c); }));
  s.erase(::std::find_if(s.rbegin(), s.rend(),
    [](char const c){ return !::std::isspace(c); }).base(), s.end());

  return s;
}

}
