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

  ::std::pair<::std::string::size_type, ::std::string::size_type> scheme_;
  ::std::pair<::std::string::size_type, ::std::string::size_type> authority_;
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
  auto& operator=(A&& a)
  {
    assign(::std::forward<A>(a));

    return *this;
  }

  operator ::std::string const& () const noexcept;

  auto scheme() const;
  auto authority() const;
  auto path() const;
  auto query() const;
  auto fragment() const;

  void assign(::std::string);

  bool is_valid() const noexcept;

  ::std::string const& to_string() const noexcept;
};

//////////////////////////////////////////////////////////////////////////////
inline url::operator ::std::string const& () const noexcept
{
  return url_;
}

//////////////////////////////////////////////////////////////////////////////
inline auto url::scheme() const
{
  return is_valid() ?
    ::std::string(url_, scheme_.first, scheme_.second) :
    ::std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline auto url::authority() const
{
  return is_valid() ?
    ::std::string(url_, authority_.first, authority_.second) :
    ::std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline auto url::path() const
{
  return is_valid() ?
    ::std::string(url_, path_.first, path_.second) :
    ::std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline auto url::query() const
{
  return is_valid() ?
    ::std::string(url_, query_.first, query_.second) :
    ::std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline auto url::fragment() const
{
  return is_valid() ?
    ::std::string(url_, fragment_.first, fragment_.second) :
    ::std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline void url::assign(::std::string u)
{
  // extract url info
  ::std::regex ex(
    R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"
  );

  ::std::cmatch what;

  auto const c_str(u.c_str());

  if ((valid_ = ::std::regex_match(c_str, what, ex)))
  {
    scheme_ = {what[2].first - c_str, what[2].second - what[2].first};
    authority_ = {what[4].first - c_str, what[4].second - what[4].first};
    path_ = {what[5].first - c_str, what[5].second - what[5].first};
    query_ = {what[7].first - c_str, what[7].second - what[7].first};
    fragment_ = {what[9].first - c_str, what[9].second - what[9].first};

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
inline ::std::string const& url::to_string() const noexcept
{
  return *this;
}

}

#endif // GENERIC_URL_HPP
