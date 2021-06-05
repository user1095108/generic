#ifndef GNR_URI_HPP
# define GNR_URI_HPP
# pragma once

#include <regex>

#include <ostream>

#include <string_view>

#include <utility>

namespace gnr
{

class uri
{
  using index_pair_t = std::pair<
    std::string::size_type,
    std::string::size_type
  >;

  struct data
  {
    std::string uri;

    index_pair_t scheme;
    index_pair_t authority;
    index_pair_t path;
    index_pair_t query;
    index_pair_t fragment;
  };

  std::unique_ptr<data> data_;

public:
  uri() = default;

  uri(uri const&);
  uri(uri&&);

  template <typename A,
    typename = std::enable_if_t<!std::is_same<std::decay_t<A>, uri>{}>
  >
  uri(A&& a)
  {
    assign(std::forward<A>(a));
  }

  uri& operator=(uri const&);
  uri& operator=(uri&&);

  template <typename A,
    typename = std::enable_if_t<!std::is_same<std::decay_t<A>, uri>{}>
  >
  uri& operator=(A&& a)
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

  auto to_string() const;
};

//////////////////////////////////////////////////////////////////////////////
inline uri::uri(uri const& other)
{
  data_.reset(new data(*other.data_));
}

//////////////////////////////////////////////////////////////////////////////
inline uri::uri(uri&& other)
{
  data_ = std::move(other.data_);
}

//////////////////////////////////////////////////////////////////////////////
inline uri& uri::operator=(uri const& other)
{
  data_.reset(new data(*other.data_));

  return *this;
}

//////////////////////////////////////////////////////////////////////////////
inline uri& uri::operator=(uri&& other)
{
  data_ = std::move(other.data_);

  return *this;
}

//////////////////////////////////////////////////////////////////////////////
inline bool uri::operator==(uri const& other) const noexcept
{
  return data_ && other.data_ ?
    other.data_->uri == data_->uri :
    data_ == other.data_;
}

//////////////////////////////////////////////////////////////////////////////
inline bool uri::operator!=(uri const& other) const noexcept
{
  return !operator==(other);
}

//////////////////////////////////////////////////////////////////////////////
inline bool uri::is_valid() const noexcept
{
  return data_ && data_->uri.size();
}

//////////////////////////////////////////////////////////////////////////////
inline auto uri::to_string() const
{
  return data_ ? data_->uri : decltype(data_->uri)();
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::scheme() const
{
  return data_ ?
    std::string_view(
      data_->uri.c_str() + data_->scheme.first,
      data_->scheme.second
    ) :
    std::string_view();
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::authority() const
{
  return data_ ?
    std::string_view(
      data_->uri.c_str() + data_->authority.first,
      data_->authority.second
    ) :
    std::string_view();
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::path() const
{
  return data_ ?
    std::string_view(
      data_->uri.c_str() + data_->path.first,
      data_->path.second
    ) :
    std::string_view();
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::query() const
{
  return data_ ?
    std::string_view(
      data_->uri.c_str() + data_->query.first,
      data_->query.second
    ) :
    std::string_view();
}

//////////////////////////////////////////////////////////////////////////////
inline std::string_view uri::fragment() const
{
  return data_ ?
    std::string_view(
      data_->uri.c_str() + data_->fragment.first,
      data_->fragment.second
    ) :
    std::string_view();
}

//////////////////////////////////////////////////////////////////////////////
inline void uri::assign(std::string const& u)
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
    data_.reset(new data());

    data_->uri = u;

    data_->scheme = {
      what[2].first - c_str,
      what[2].second - what[2].first
    };

    data_->authority = {
      what[4].first - c_str,
      what[4].second - what[4].first
    };

    data_->path = {
      what[5].first - c_str,
      what[5].second - what[5].first
    };

    data_->query = {
      what[7].first - c_str,
      what[7].second - what[7].first
    };

    data_->fragment = {
      what[9].first - c_str,
      what[9].second - what[9].first
    };
  }
  else
  {
    data_.reset();
  }
}

inline std::ostream& operator<<(std::ostream& os, uri const& uri)
{
  return os << uri.to_string();
}

}

#endif // GNR_URI_HPP
