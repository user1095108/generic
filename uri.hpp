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

  using index_pair_t = std::pair<
    std::string::size_type,
    std::string::size_type
  >;

  index_pair_t scheme_;
  index_pair_t authority_;
  index_pair_t path_;
  index_pair_t query_;
  index_pair_t fragment_;

public:
  template <typename A>
  uri(A&& a)
  {
    assign(std::forward<A>(a));
  }

  template <typename A>
  auto& operator=(A&& a)
  {
    assign(std::forward<A>(a));

    return *this;
  }

  bool operator==(uri const&) const noexcept;
  bool operator!=(uri const&) const noexcept;

  operator std::string const& () const noexcept;

  std::string_view scheme() const;
  std::string_view authority() const;
  std::string_view path() const;
  std::string_view query() const;
  std::string_view fragment() const;

  void assign(std::string const&);

  bool is_valid() const noexcept;

  std::string const& to_string() const noexcept;
};

//////////////////////////////////////////////////////////////////////////////
inline bool uri::operator==(uri const& other) const noexcept
{
  return other.to_string() == uri_;
}

//////////////////////////////////////////////////////////////////////////////
inline bool uri::operator!=(uri const& other) const noexcept
{
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////////////
inline bool uri::is_valid() const noexcept
{
  return uri_.size();
}

//////////////////////////////////////////////////////////////////////////////
inline std::string const& uri::to_string() const noexcept
{
  return uri_;
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::scheme() const
{
  return std::string_view(
    uri_.c_str() + scheme_.first,
    scheme_.second
  );
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::authority() const
{
  return std::string_view(
    uri_.c_str() + authority_.first,
    authority_.second
  );
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::path() const
{
  return std::string_view(
    uri_.c_str() + path_.first,
    path_.second
  );
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::query() const
{
  return std::string_view(
    uri_.c_str() + query_.first,
    query_.second
  );
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::fragment() const
{
  return std::string_view(
    uri_.c_str() + fragment_.first,
    fragment_.second
  );
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
    scheme_ = {
      what[2].first - c_str,
      what[2].second - what[2].first
    };

    authority_ = {
      what[4].first - c_str,
      what[4].second - what[4].first
    };

    path_ = {
      what[5].first - c_str,
      what[5].second - what[5].first
    };

    query_ = {
      what[7].first - c_str,
      what[7].second - what[7].first
    };

    fragment_ = {
      what[9].first - c_str,
      what[9].second - what[9].first
    };
  }
  else
  {
    uri_.clear();
    uri_.shrink_to_fit();

    scheme_ = {0, 0};
    authority_ = {0, 0};
    path_ = {0, 0};
    query_ = {0, 0};
    fragment_ = {0, 0};
  }
}

}

#endif // GNR_URI_HPP
