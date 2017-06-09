#ifndef GNR_URI_HPP
# define GNR_URI_HPP
# pragma once

#include <regex>

#include <string_view>

#include <utility>

namespace gnr
{

class uri
{
  std::string uri_;

  std::string_view scheme_;
  std::string_view authority_;
  std::string_view path_;
  std::string_view query_;
  std::string_view fragment_;

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

  auto& scheme() const;
  auto& authority() const;
  auto& path() const;
  auto& query() const;
  auto& fragment() const;

  void assign(std::string const&);
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
inline auto& uri::scheme() const
{
  return scheme_;
}

//////////////////////////////////////////////////////////////////////////////
inline auto& uri::authority() const
{
  return authority_;
}

//////////////////////////////////////////////////////////////////////////////
inline auto& uri::path() const
{
  return path_;
}

//////////////////////////////////////////////////////////////////////////////
inline auto& uri::query() const
{
  return query_;
}

//////////////////////////////////////////////////////////////////////////////
inline auto& uri::fragment() const
{
  return fragment_;
}

//////////////////////////////////////////////////////////////////////////////
inline void uri::assign(std::string const& u)
{
  //rfc3986
  static std::regex const ex{
    R"(^(([^:/?#]+):)?(//([^/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)"
  };

  uri_ = u;

  // extract uri info
  std::cmatch what;

  auto const c_str(uri_.c_str());

  if (std::regex_match(c_str, what, ex))
  {
    scheme_ = std::string_view(what[2].first,
      what[2].second - what[2].first
    );
    authority_ = std::string_view(what[4].first,
      what[4].second - what[4].first
    );
    path_ = std::string_view(what[5].first,
      what[5].second - what[5].first
    );
    query_ = std::string_view(what[7].first,
      what[7].second - what[7].first
    );
    fragment_ = std::string_view(what[9].first,
      what[9].second - what[9].first
    );
  }
  else
  {
    uri_.clear();
    uri_.shrink_to_fit();

    scheme_ = {};
    authority_ = {};
    path_ = {};
    query_ = {};
    fragment_ = {};
  }
}

}

#endif // GNR_URI_HPP
