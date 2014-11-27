#ifndef VANY_HPP
# define VANY_HPP
# pragma once

#include <cassert>

#include <cstdint>

#include <stdexcept>

#include <typeinfo>

#include <type_traits> 

#include <utility>

namespace generic
{

template <typename T>
using remove_cvr = ::std::remove_cv<
  typename ::std::remove_reference<T>::type
>;

using typeid_t = void const*;

template <typename T>
typeid_t type_id() noexcept
{
  static char const type_id{};

  return &type_id;
}

template <typename ...U>
class vany
{
  struct placeholder;

  placeholder const* content() const noexcept
  {
    return reinterpret_cast<placeholder const*>(&store_);
  }

  placeholder* content() noexcept
  {
    return reinterpret_cast<placeholder*>(&store_);
  }

  template <typename A, typename ...B>
  struct max_type_size : ::std::conditional<
      (sizeof(A) > max_type_size<B...>{}),
      ::std::integral_constant<::std::size_t, sizeof(A)>,
      max_type_size<B...>
    >::type
  {
  };

  template <typename A, typename B>
  struct max_type_size<A, B> : ::std::conditional<
      (sizeof(A) > sizeof(B)),
      ::std::integral_constant<::std::size_t, sizeof(A)>,
      ::std::integral_constant<::std::size_t, sizeof(B)>
    >::type
  {
  };

  template <typename A>
  struct max_type_size<A> :
    ::std::integral_constant<::std::size_t, sizeof(A)>
  {
  };

public:
  vany() = default;

  vany(vany const& other)
  {
    //content(other.content ? other.content->cloner_(other.content) : nullptr)
  }

  vany(vany&& other) noexcept { *this = ::std::move(other); }

  template<typename ValueType,
    typename = typename ::std::enable_if<
      !::std::is_same<vany, typename ::std::decay<ValueType>::type>{}
    >::type
  >
  vany(ValueType&& value)
  {
    new (&store_) holder<typename remove_cvr<ValueType>::type>(
      ::std::forward<ValueType>(value));
  }

  ~vany() { delete content(); }

public: // modifiers
  void clear() { swap(vany()); }

  bool empty() const noexcept { return !*this; }

  void swap(vany& other) noexcept
  {
    //::std::swap(content, other.content);
  }

  void swap(vany&& other) noexcept
  {
    //::std::swap(content, other.content);
  }

  vany& operator=(vany const& rhs) { return *this = vany(rhs); }

  vany& operator=(vany&& rhs) noexcept { swap(rhs); return *this; }

  template<typename ValueType,
    typename = typename ::std::enable_if<
      !::std::is_same<vany, typename remove_cvr<ValueType>::type>{}
    >::type
  >
  vany& operator=(ValueType&& rhs)
  {
    return *this = vany(::std::forward<ValueType>(rhs));
  }

public: // queries

  explicit operator bool() const noexcept { return content(); }

  typeid_t type() const noexcept { return type_id(); }

  typeid_t type_id() const noexcept
  {
    return content() ? content()->type_id_ : ::generic::type_id<void>();
  }

public: // get

  template <typename ValueType>
  ValueType cget() const
  {
    return get<ValueType>();
  }

  template <typename ValueType>
  ValueType get() const
  {
    using nonref = typename ::std::remove_reference<ValueType>::type;

    return const_cast<vany*>(this)->get<nonref const&>();
  }

  template <typename ValueType>
  ValueType get()
  {
    using nonref = typename ::std::remove_reference<ValueType>::type;

#ifndef NDEBUG
    if (content() && (type_id() ==
      ::generic::type_id<typename remove_cvr<ValueType>::type>()))
    {
      return static_cast<vany::holder<nonref>*>(content())->held;
    }
    else
    {
      throw ::std::bad_typeid();
    }
#else
    return static_cast<vany::holder<nonref>*>(content)->held;
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
    return begin(array[0]);
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
    template <class T, typename V = ValueType>
    holder(T&& value,
      typename ::std::enable_if<
        !::std::is_array<V>{} &&
        !::std::is_copy_constructible<V>{}
      >::type* = nullptr) :
      placeholder(::generic::type_id<ValueType>(), throwing_cloner),
      held(::std::forward<T>(value))
    {
    }

    template <class T, typename V = ValueType>
    holder(T&& value,
      typename ::std::enable_if<
        !::std::is_array<V>{} &&
        ::std::is_copy_constructible<V>{}
      >::type* = nullptr) :
      placeholder(::generic::type_id<ValueType>(), cloner),
      held(::std::forward<T>(value))
    {
    }

    template <class T, typename V = ValueType>
    holder(T&& value,
      typename ::std::enable_if<
        ::std::is_array<V>{} &&
        !::std::is_copy_assignable<
          typename ::std::remove_all_extents<V>::type
        >{} &&
        ::std::is_move_assignable<
          typename ::std::remove_all_extents<V>::type
        >{}
      >::type* = nullptr) :
      placeholder(::generic::type_id<ValueType>(), throwing_cloner)
    {
      ::std::copy(::std::make_move_iterator(begin(value)),
        ::std::make_move_iterator(end(value)),
        begin(held));
    }

    template <class T, typename V = ValueType>
    holder(T&& value,
      typename ::std::enable_if<
        ::std::is_array<V>{} &&
        ::std::is_copy_assignable<
          typename ::std::remove_all_extents<V>::type
        >{}
      >::type* = nullptr) :
      placeholder(::generic::type_id<ValueType>(), cloner)
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
    ValueType held;
  };

private: // representation

  template<typename ValueType>
  friend ValueType* any_cast(vany*) noexcept;

  template<typename ValueType>
  friend ValueType* unsafe_any_cast(vany*) noexcept;

  ::std::aligned_storage<max_type_size<U...>{}> store_;
};

template<typename ValueType, typename ...U>
inline ValueType* unsafe_any_cast(
  vany<U...>* const operand) noexcept
{
  return &static_cast<typename vany<U...>::template holder<ValueType>*>(operand->content())->held;
}

template<typename ValueType, typename ...U>
inline ValueType const* unsafe_any_cast(
  vany<U...> const* const operand) noexcept
{
  return unsafe_any_cast<ValueType>(const_cast<vany<U...>*>(operand));
}

template<typename ValueType, typename ...U>
inline ValueType* any_cast(vany<U...>* const operand) noexcept
{
  return operand &&
    operand->type_id() == type_id<typename remove_cvr<ValueType>::type>() ?
    &static_cast<typename vany<U...>::template holder<ValueType>*>(
      operand->content())->held :
    nullptr;
}

template<typename ValueType, typename ...U>
inline ValueType const* any_cast(vany<U...> const* const operand) noexcept
{
  return any_cast<ValueType>(const_cast<vany<U...>*>(operand));
}

template<typename ValueType, typename ...U>
inline ValueType any_cast(vany<U...>& operand)
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

template<typename ValueType, typename ...U>
inline ValueType any_cast(vany<U...> const& operand)
{
  using nonref = typename ::std::remove_reference<ValueType>::type;

  return any_cast<nonref const&>(const_cast<vany<U...>&>(operand));
}

}

#endif // VANY_HPP
