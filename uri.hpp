#ifndef GENERIC_URI_HPP
# define GENERIC_URI_HPP
# pragma once

#include <regex>

#include <string>

#include <utility>

namespace generic
{

class uri
{
  std::string uri_;

  std::pair<std::string::size_type, std::string::size_type> scheme_;
  std::pair<std::string::size_type, std::string::size_type> authority_;
  std::pair<std::string::size_type, std::string::size_type> path_;
  std::pair<std::string::size_type, std::string::size_type> query_;
  std::pair<std::string::size_type, std::string::size_type> fragment_;

public:
  template <typename A>
  explicit uri(A&& a)
  {
    assign(std::forward<A>(a));
  }

  template <typename A>
  auto& operator=(A&& a)
  {
    assign(std::forward<A>(a));

    return *this;
  }

  bool is_valid() const noexcept;

  operator std::string const& () const noexcept;

  std::string const& to_string() const noexcept;

  auto scheme() const;
  auto authority() const;
  auto path() const;
  auto query() const;
  auto fragment() const;

  void assign(std::string);
};

//////////////////////////////////////////////////////////////////////////////
inline bool uri::is_valid() const noexcept
{
  return uri_.size();
}

//////////////////////////////////////////////////////////////////////////////
inline uri::operator std::string const& () const noexcept
{
  return uri_;
}

//////////////////////////////////////////////////////////////////////////////
inline std::string const& uri::to_string() const noexcept
{
  return *this;
}

//////////////////////////////////////////////////////////////////////////////
inline auto uri::scheme() const
{
  return is_valid() ?
    std::string(uri_, scheme_.first, scheme_.second) :
    std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline auto uri::authority() const
{
  return is_valid() ?
    std::string(uri_, authority_.first, authority_.second) :
    std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline auto uri::path() const
{
  return is_valid() ?
    std::string(uri_, path_.first, path_.second) :
    std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline auto uri::query() const
{
  return is_valid() ?
    std::string(uri_, query_.first, query_.second) :
    std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline auto uri::fragment() const
{
  return is_valid() ?
    std::string(uri_, fragment_.first, fragment_.second) :
    std::string();
}

//////////////////////////////////////////////////////////////////////////////
inline void uri::assign(std::string u)
{
  //rfc3986
  static std::regex const ex{
    R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"
  };

  // extract uri info
  std::cmatch what;

  auto const c_str(u.c_str());

  if (std::regex_match(c_str, what, ex))
  {
    scheme_ = {what[2].first - c_str, what[2].second - what[2].first};
    authority_ = {what[4].first - c_str, what[4].second - what[4].first};
    path_ = {what[5].first - c_str, what[5].second - what[5].first};
    query_ = {what[7].first - c_str, what[7].second - what[7].first};
    fragment_ = {what[9].first - c_str, what[9].second - what[9].first};

    uri_ = std::move(u);
  }
  else
  {
    uri_.clear();
    uri_.shrink_to_fit();
  }
}

}

#endif // GENERIC_URI_HPP
