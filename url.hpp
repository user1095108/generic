#ifndef GENERIC_URL_HPP
# define GENERIC_URL_HPP
# pragma once

#include <regex>

#include <string>

#include <utility>

namespace generic
{

class url
{
  ::std::string url_;

  ::std::pair<::std::string::size_type, ::std::string::size_type> protocol_;
  ::std::pair<::std::string::size_type, ::std::string::size_type> domain_;
  ::std::pair<::std::string::size_type, ::std::string::size_type> port_;
  ::std::pair<::std::string::size_type, ::std::string::size_type> path_;
  ::std::pair<::std::string::size_type, ::std::string::size_type> query_;
  ::std::pair<::std::string::size_type, ::std::string::size_type> fragment_;

  bool valid_{};

public:
  template <typename A>
  explicit url(A&& a)
  {
    assign(::std::forward<A>(a));
  }

  template <typename A>
  auto& operator=(A&& u)
  {
    assign(::std::forward<A>(a));

    return *this;
  }

  operator ::std::string const& () const
  {
    return url_;
  }

  void assign(::std::string);

  bool is_valid() const noexcept;

  ::std::string const& to_string() const;
};

//////////////////////////////////////////////////////////////////////////////
inline void url::assign(::std::string u)
{
  // extract url info
  ::std::regex ex("(.+)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");

  ::std::cmatch what;

  auto const c_str(u.c_str());

  if ((valid_ = ::std::regex_match(c_str, what, ex)))
  {
/*
    cout << "protocol: " << string(what[1].first, what[1].second) << endl;
    cout << "domain:   " << string(what[2].first, what[2].second) << endl;
    cout << "port:     " << string(what[3].first, what[3].second) << endl;
    cout << "path:     " << string(what[4].first, what[4].second) << endl;
    cout << "query:    " << string(what[5].first, what[5].second) << endl;
    cout << "fragment: " << string(what[6].first, what[6].second) << endl;
*/
    protocol_ = {what[1].first - c_str, what[1].second - what[1].first};
    domain_ = {what[2].first - c_str, what[2].second - what[2].first};
    port_ = {what[3].first - c_str, what[3].second - what[3].first};
    path_ = {what[4].first - c_str, what[4].second - what[4].first};
    query_ = {what[5].first - c_str, what[5].second - what[5].first};
    fragment_ = {what[6].first - c_str, what[6].second - what[6].first};

    url_ = ::std::move(u);
  }
  else
  {
    url_.clear();
    url_.shrink_to_fit();
  }
}

//////////////////////////////////////////////////////////////////////////////
inline bool url::is_valid() const noexcept
{
  return valid_;
}

//////////////////////////////////////////////////////////////////////////////
inline ::std::string const& to_string() const
{
  return *this;
}

}

#endif // GENERIC_URL_HPP
