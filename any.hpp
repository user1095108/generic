#ifndef ANY_HPP
# define ANY_HPP
# pragma once

#include <cassert>

#include <cstdint>

#include <stdexcept>

#include <typeinfo>

#include <type_traits> 

#include <utility>

namespace generic
{

class any
{
public:
  using typeid_t = void (*)();

  template <typename T>
  static constexpr typeid_t type_id() noexcept
  {
    return typeid_t(type_id<T>);
  }

  any() = default;

  any(any const& other) :
    content(other.content ? other.content->clone() : nullptr)
  {
  }

  any(any&& other) noexcept { *this = ::std::move(other); }

  template<typename ValueType,
    typename = typename ::std::enable_if<
      !::std::is_same<any, typename ::std::decay<ValueType>::type>::value
    >::type
  >
  any(ValueType&& value) :
    content(new holder<typename ::std::decay<ValueType>::type>(
      ::std::forward<ValueType>(value)))
  {
  }

  ~any() { delete content; }

public: // modifiers
  void clear() { swap(any()); }

  bool empty() const noexcept { return !*this; }

  void swap(any& other) noexcept { ::std::swap(content, other.content); }

  void swap(any&& other) noexcept { ::std::swap(content, other.content); }

  any& operator=(any const& rhs) { return *this = any(rhs); }

  any& operator=(any&& rhs) noexcept
  {
    content = rhs.content;
    rhs.content = nullptr;

    return *this;
  }

  template<typename ValueType,
    typename = typename ::std::enable_if<
      !::std::is_same<any, typename ::std::decay<ValueType>::type>{}
    >::type
  >
  any& operator=(ValueType&& rhs)
  {
    return *this = any(::std::forward<ValueType>(rhs));
  }

public: // queries

  explicit operator bool() const noexcept { return content; }

  typeid_t type_id() const noexcept
  {
    return content ? content->type_id_ : type_id<void>();
  }

private: // types

  struct placeholder
  {
    placeholder() = default;

    virtual ~placeholder() = default;

    virtual placeholder* clone() const = 0;

    typeid_t type_id_;
  };

  template <typename ValueType, typename = void>
  struct holder : public placeholder
  {
  public: // constructor
    template <class T> holder(T&& value) :
      held(type_id_ = any::type_id<ValueType>(), ::std::forward<T>(value))
    {
    }

    holder& operator=(holder const&) = delete;

    placeholder* clone() const final { throw ::std::logic_error(""); }

  public:
    ValueType held;
  };

  template <typename ValueType>
  struct holder<
    ValueType,
    typename ::std::enable_if<
      ::std::is_copy_constructible<ValueType>::value
    >::type
  > : public placeholder
  {
  public: // constructor
    template <class T>
    holder(T&& value) :
      held(type_id_ = any::type_id<ValueType>(), ::std::forward<T>(value))
    {
    }

    holder& operator=(holder const&) = delete;

    placeholder* clone() const final { return new holder<ValueType>(held); }

  public:
    ValueType held;
  };

private: // representation

  template<typename ValueType>
  friend ValueType* any_cast(any*) noexcept;

  template<typename ValueType>
  friend ValueType* unsafe_any_cast(any*) noexcept;

  placeholder* content{};
};

template<typename ValueType>
inline ValueType* unsafe_any_cast(any* const operand) noexcept
{
  return &static_cast<any::holder<ValueType>*>(operand->content)->held;
}

template<typename ValueType>
inline ValueType const* unsafe_any_cast(any const* const operand) noexcept
{
  return unsafe_any_cast<ValueType>(const_cast<any*>(operand));
}

template<typename ValueType>
inline ValueType* any_cast(any* const operand) noexcept
{
  return operand &&
    (operand->type_id() ==
      any::type_id<typename ::std::decay<ValueType>::type>()) ?
    &static_cast<any::holder<ValueType>*>(operand->content)->held :
    nullptr;
}

template<typename ValueType>
inline ValueType const* any_cast(any const* const operand) noexcept
{
  return any_cast<ValueType>(const_cast<any*>(operand));
}

template<typename ValueType>
inline ValueType any_cast(any& operand)
{
  using nonref = typename ::std::remove_reference<ValueType>::type;

#ifndef NDEBUG
  auto const result(any_cast<nonref>(&operand));

  if (result)
  {
    return *result;
  }
  else
  {
    throw ::std::bad_cast();
  }
#else
  return *unsafe_any_cast<nonref>(&operand);
#endif // NDEBUG
}

template<typename ValueType>
inline ValueType any_cast(any const& operand)
{
  using nonref = typename ::std::remove_reference<ValueType>::type;

  return any_cast<nonref const&>(const_cast<any&>(operand));
}

}

#endif // ANY_HPP
