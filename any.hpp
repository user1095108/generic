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
  template <typename T>
  using remove_cvr = ::std::remove_cv<
    typename ::std::remove_reference<T>::type
  >;

  using typeid_t = void const*;

  template <typename T>
  static typeid_t type_id() noexcept
  {
    static struct { } const type_id;

    return &type_id;
  }

  any() = default;

  any(any const& other) :
    content(other.content ? other.content->cloner_(other.content) : nullptr)
  {
  }

  any(any&& other) noexcept { *this = ::std::move(other); }

  template<typename ValueType,
    typename = typename ::std::enable_if<
      !::std::is_same<
        any, typename ::std::decay<ValueType>::type
      >{}
    >::type
  >
  any(ValueType&& value) :
    content(new holder<typename remove_cvr<ValueType>::type>(
      ::std::forward<ValueType>(value)))
  {
  }

  ~any() { delete content; }

public: // modifiers
  void clear() { swap(any()); }

  bool empty() const noexcept { return !*this; }

  void swap(any& other) noexcept { ::std::swap(content, other.content); }

  void swap(any&& other) noexcept { ::std::swap(content, other.content); }

  any& operator=(any const& rhs)
  {
    return content == rhs.content ? *this : *this = any(rhs);
  }

  any& operator=(any&& rhs) noexcept { swap(rhs); return *this; }

  template<typename ValueType,
    typename = typename ::std::enable_if<
      !::std::is_same<
        any, typename ::std::decay<ValueType>::type
      >{}
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

  auto type() const noexcept -> decltype(type_id()) { return type_id(); }

public: // get

  template <typename ValueType>
  ValueType cget() const
  {
    return get<ValueType>();
  }

  template <typename ValueType>
  ValueType get() const
  {
    using nonref = typename remove_cvr<ValueType>::type;

    return const_cast<any*>(this)->get<nonref const&>();
  }

  template <typename ValueType>
  ValueType get()
  {
    using nonref = typename remove_cvr<ValueType>::type;

#ifndef NDEBUG
    if (content && (type_id() ==
      type_id<typename remove_cvr<ValueType>::type>()))
    {
      return static_cast<any::holder<nonref>*>(content)->held;
    }
    else
    {
      throw ::std::bad_typeid();
    }
#else
    return static_cast<any::holder<nonref>*>(content)->held;
#endif // NDEBUG
  }

private: // types

  template <typename T>
  static constexpr T* begin(T& value) noexcept
  {
    return &value;
  }

  template <typename T, ::std::size_t N>
  static constexpr typename ::std::remove_all_extents<T>::type*
  begin(T (&array)[N]) noexcept
  {
    return begin(*array);
  }

  template <typename T>
  static constexpr T* end(T& value) noexcept
  {
    return &value + 1;
  }

  template <typename T, ::std::size_t N>
  static constexpr typename ::std::remove_all_extents<T>::type*
  end(T (&array)[N]) noexcept
  {
    return end(array[N - 1]);
  }

  struct placeholder
  {
    typeid_t const type_id_;

    placeholder* (* const cloner_)(placeholder*);

    virtual ~placeholder() = default;

  protected:

    placeholder(typeid_t const ti, decltype(cloner_) const c) noexcept :
      type_id_(ti),
      cloner_(c)
    {
    }
  };

  template <typename ValueType>
  struct holder : public placeholder
  {
  public: // constructor
    template <class T, typename U = ValueType>
    holder(T&& value,
      typename ::std::enable_if<
        !::std::is_array<U>{} &&
        !::std::is_copy_constructible<U>{}
      >::type* = nullptr) :
      placeholder(type_id<ValueType>(), throwing_cloner),
      held(::std::forward<T>(value))
    {
    }

    template <class T, typename U = ValueType>
    holder(T&& value,
      typename ::std::enable_if<
        !::std::is_array<U>{} &&
        ::std::is_copy_constructible<U>{}
      >::type* = nullptr) :
      placeholder(type_id<ValueType>(), cloner),
      held(::std::forward<T>(value))
    {
    }

    template <class T, typename U = ValueType>
    holder(T&& value,
      typename ::std::enable_if<
        ::std::is_array<U>{} &&
        ::std::is_move_assignable<
          typename ::std::remove_const<
            typename ::std::remove_all_extents<U>::type
          >::type
        >{} &&
        ::std::is_rvalue_reference<T&&>{}
      >::type* = nullptr) :
      placeholder(type_id<ValueType>(), throwing_cloner)
    {
      ::std::copy(::std::make_move_iterator(begin(value)),
        ::std::make_move_iterator(end(value)),
        begin(held));
    }

    template <class T, typename U = ValueType>
    holder(T&& value,
      typename ::std::enable_if<
        ::std::is_array<U>{} &&
        ::std::is_copy_assignable<
          typename ::std::remove_const<
            typename ::std::remove_all_extents<U>::type
          >::type
        >{} &&
        !::std::is_rvalue_reference<T&&>{}
      >::type* = nullptr) :
      placeholder(type_id<ValueType>(), cloner)
    {
      ::std::copy(begin(value), end(value), begin(held));
    }

    holder& operator=(holder const&) = delete;

    static placeholder* cloner(placeholder* const base)
    {
      return new holder<ValueType>(static_cast<holder*>(base)->held);
    }

    static placeholder* throwing_cloner(placeholder* const)
    {
      throw ::std::logic_error("");
    }

  public:
    typename ::std::remove_const<ValueType>::type held;
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
      any::type_id<typename any::remove_cvr<ValueType>::type>()) ?
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
  using nonref = typename any::remove_cvr<ValueType>::type;

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
  using nonref = typename any::remove_cvr<ValueType>::type;

  return any_cast<nonref const&>(const_cast<any&>(operand));
}

}

#endif // ANY_HPP
