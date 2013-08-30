#pragma once
#ifndef LIGHTPTR_HPP
# define LIGHTPTR_HPP

#include <cassert>

#include <atomic>

#include <memory>

#include <utility>

#include <type_traits>

namespace detail
{
  using counter_type = ::std::size_t;

  using atomic_type = ::std::atomic<counter_type>;

  using deleter_type = void (*)(void*);

  template <typename U>
  struct ref_type
  {
    using type = U&;
  };

  template <>
  struct ref_type<void>
  {
    using type = void;
  };

  inline void dec_ref(atomic_type* const counter_ptr,
    void* const ptr, deleter_type const deleter)
  {
    if (counter_ptr && !--*counter_ptr)
    {
      delete counter_ptr;

      deleter(ptr);
    }
    // else do nothing
  }

  inline void inc_ref(atomic_type* const counter_ptr)
  {
    assert(counter_ptr);
    ++*counter_ptr;
  }
}

template <typename T>
struct light_ptr
{
  template <typename U>
  struct remove_array
  {
    using type = U;
  };

  template <typename U>
  struct remove_array<U[]>
  {
    using type = U;
  };

  using counter_type = ::detail::counter_type;

  using deleter_type = ::detail::deleter_type;

  using element_type = typename remove_array<T>::type;

  constexpr light_ptr() = default;

  explicit light_ptr(element_type* const p,
    deleter_type const d = default_deleter)
  {
    reset(p, d);
  }

  ~light_ptr() { ::detail::dec_ref(counter_ptr_, ptr_, deleter_); }

  light_ptr(light_ptr const& other) { *this = other; }

  light_ptr(light_ptr&& other) noexcept { *this = ::std::move(other); }

  light_ptr& operator=(light_ptr const& rhs)
  {
    if (*this != rhs)
    {
      ::detail::dec_ref(counter_ptr_, ptr_, deleter_);

      counter_ptr_ = rhs.counter_ptr_;

      ptr_ = rhs.ptr_;

      deleter_ = rhs.deleter_;

      ::detail::inc_ref(counter_ptr_);
    }
    // else do nothing

    return *this;
  }

  light_ptr& operator=(light_ptr&& rhs) noexcept
  {
    counter_ptr_ = rhs.counter_ptr_;
    ptr_ = rhs.ptr_;
    deleter_ = rhs.deleter_;

    rhs.counter_ptr_ = nullptr;

    return *this;
  }

  bool operator<(light_ptr const& rhs) const noexcept
  {
    return get() < rhs.get();
  }

  bool operator==(light_ptr const& rhs) const noexcept
  {
    return counter_ptr_ == rhs.counter_ptr_;
  }

  bool operator!=(light_ptr const& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  explicit operator bool() const noexcept { return counter_ptr_; }

  typename ::detail::ref_type<T>::type
  operator*() const noexcept
  {
    return *static_cast<T*>(static_cast<void*>(ptr_));
  }

  T* operator->() const noexcept
  {
    return static_cast<T*>(static_cast<void*>(ptr_));
  }

  element_type* get() const noexcept { return ptr_; }

  void reset() { reset(nullptr); }

  void reset(::std::nullptr_t const p)
  {
    ::detail::dec_ref(counter_ptr_, ptr_, deleter_);

    counter_ptr_ = nullptr;

    ptr_ = nullptr;
  }

  void reset(element_type* const p,
    deleter_type const d = default_deleter)
  {
    ::detail::dec_ref(counter_ptr_, ptr_, deleter_);

    counter_ptr_ = new atomic_type(counter_type(1));

    ptr_ = p;

    deleter_ = d;
  }

  void swap(light_ptr& other) noexcept
  {
    ::std::swap(counter_ptr_, other.counter_ptr_);
    ::std::swap(ptr_, other.ptr_);
    ::std::swap(deleter_, other.deleter_);
  }

  bool unique() const noexcept { return counter_type(1) == use_count(); }

  counter_type use_count() const noexcept
  {
    return counter_ptr_ ? counter_ptr_->load() : counter_type{};
  }

private:
  static void default_deleter(void* const p)
  {
    ::std::default_delete<T>()(static_cast<element_type*>(p));
  }

  using atomic_type = ::detail::atomic_type;

private:
  atomic_type* counter_ptr_{};

  element_type* ptr_{};

  deleter_type deleter_;
};

template<class T, class... Args>
inline light_ptr<T> make_light(Args&&... args)
{
  return light_ptr<T>(new T(::std::forward<Args>(args)...));
}

namespace std
{
  template <typename T>
  struct hash
  {
    size_t operator()(light_ptr<T> const& l) const noexcept
    {
      return hash<typename light_ptr<T>::element_type*>(l.get());
    }
  };
}

#endif // LIGHTPTR_HPP
