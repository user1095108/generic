#ifndef ANY_HPP
# define ANY_HPP

#include <cassert>

#include <stdexcept>

#include <typeinfo>

#include <type_traits> 

#include <utility>

namespace generic
{

class any
{
public:
  any() : content(nullptr) { }

  explicit any(any const& other)
    : content(other.content ? other.content->clone() : nullptr)
  {
  }

  explicit any(any&& other) { *this = std::move(other); }

  template<typename ValueType,
    typename = typename std::enable_if<
      !std::is_same<any, typename std::decay<ValueType>::type>::value
    >::type
  >
  any(ValueType&& value)
    : content(new holder<typename std::remove_reference<ValueType>::type>(
        std::forward<ValueType>(value)))
  {
  }

  ~any() { delete content; }

public: // modifiers

  void swap(any& other) { std::swap(content, other.content); }

  any& operator=(any const& rhs) { return *this = any(rhs); }

  any& operator=(any&& rhs)
  {
    content = rhs.content;
    rhs.content = nullptr;

    return *this;
  }

  template<typename ValueType,
    typename = typename std::enable_if<
      !std::is_same<any, typename std::remove_const<
        typename std::remove_reference<ValueType>::type>::type>::value
    >::type
  >
  any& operator=(ValueType&& rhs)
  {
    return *this = any(std::forward<ValueType>(rhs));
  }

public: // queries

  explicit operator bool() const { return content; }

  std::type_info const& type() const
  {
    return content ? content->type() : typeid(void);
  }

private: // types

  struct placeholder
  {
    placeholder() = default;

    template <typename T> placeholder(T&&) = delete;

    virtual ~placeholder() noexcept { }

    template <typename T> placeholder& operator=(T&&) = delete;

    virtual placeholder* clone() const = 0;

    virtual std::type_info const& type() const = 0;
  };

  template<typename ValueType, typename = void>
  struct holder : public placeholder
  {
  public: // constructor
    template <class T>
    holder(T&& value) : held(std::forward<T>(value)) { }

    placeholder* clone() const final { throw std::invalid_argument(""); }

  public: // queries
    std::type_info const& type() const { return typeid(ValueType); }

  public:
    ValueType held;
  };

  template<typename ValueType>
  struct holder<
    ValueType,
    typename std::enable_if<
      std::is_copy_constructible<ValueType>::value
    >::type
  > : public placeholder
  {
  public: // constructor
    template <class T>
    holder(T&& value) : held(std::forward<T>(value)) { }

    placeholder* clone() const final { return new holder<ValueType>(held); }

  public: // queries
    std::type_info const& type() const { return typeid(ValueType); }

  public:
    ValueType held;
  };

private: // representation

  template<typename ValueType>
  friend ValueType* any_cast(any*);

  template<typename ValueType>
  friend ValueType* unsafe_any_cast(any *);

  placeholder* content;
};

template<typename ValueType>
inline ValueType* unsafe_any_cast(any* const operand)
{
  return &static_cast<any::holder<ValueType>*>(operand->content)->held;
}

template<typename ValueType>
inline ValueType const* unsafe_any_cast(any const* const operand)
{
  return unsafe_any_cast<ValueType>(const_cast<any*>(operand));
}

template<typename ValueType>
inline ValueType* any_cast(any* const operand)
{
  return operand && (&operand->type() == &typeid(ValueType))
    ? &static_cast<any::holder<ValueType>*>(operand->content)->held
    : nullptr;
}

template<typename ValueType>
inline ValueType const* any_cast(any const* const operand)
{
  return any_cast<ValueType>(const_cast<any*>(operand));
}

template<typename ValueType>
inline ValueType any_cast(any& operand)
{
  typedef typename std::remove_reference<ValueType>::type nonref;

#ifndef NDEBUG
  nonref* const result(any_cast<nonref>(&operand));

  if (!result)
  {
    throw std::bad_cast();
  }
  // else do nothing

  return *result;
#else
  return *unsafe_any_cast<nonref>(&operand);
#endif // NDEBUG
}

template<typename ValueType>
inline ValueType any_cast(any const& operand)
{
  typedef typename std::remove_reference<ValueType>::type nonref;

  return any_cast<nonref const&>(const_cast<any&>(operand));
}

}

#endif // ANY_HPP
