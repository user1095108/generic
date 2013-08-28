#pragma once
#ifndef LIGHTPTR_HPP
# define LIGHTPTR_HPP

#include <cassert>

#include <atomic>

#include <memory>

#include <type_traits>

namespace detail
{
  template <typename T>
  struct remove_array
  {
    using type = T;
  };

  template <typename T>
  struct remove_array<T[]>
  {
    using type = T;
  };
}

template <typename T,
  class D = typename std::default_delete<T> >
struct light_ptr
{
  using counter_type = std::size_t;

  light_ptr() = default;

  explicit light_ptr(typename ::detail::remove_array<T>::type* const p) :
    counter_ptr_(new atomic_type(counter_type(1))),
    ptr_(p)
  {
  }

  ~light_ptr() { dec_ref(); }

  light_ptr(light_ptr const& other) { *this = other; }

  light_ptr(light_ptr&& other) noexcept { *this = std::move(other); }

  light_ptr& operator=(light_ptr const& other)
  {
    if (*this != other)
    {
      dec_ref();

      if ((counter_ptr_ = other.counter_ptr_))
      {
        ptr_ = other.ptr_;

        inc_ref();
      }
      // else do nothing
    }
    // else do nothing

    return *this;
  }

  light_ptr& operator=(light_ptr&& rhs) noexcept
  {
    counter_ptr_ = rhs.counter_ptr_;
    ptr_ = rhs.ptr_;

    rhs.counter_ptr_ = nullptr;

    return *this;
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

  T& operator*() const noexcept { return *ptr_; }

  T* operator->() const noexcept { return ptr_; }

  T* get() const noexcept { return counter_ptr_ ? ptr_ : nullptr; }

  void reset(T* const p = nullptr)
  {
    dec_ref();

    if (p)
    {
      counter_ptr_ = new atomic_type(counter_type(1));

      ptr_ = p;
    }
    else
    {
      counter_ptr_ = nullptr;
    }
    // else do nothing
  }

  bool unique() const noexcept { return counter_type(1) == use_count(); }

  counter_type use_count() const noexcept
  {
    return counter_ptr_ ? counter_ptr_->load() : counter_type{};
  }

private:
  void dec_ref()
  {
    using deleter_type = D;

    if (counter_ptr_ && *counter_ptr_ && !--*counter_ptr_)
    {
      deleter_type()(ptr_);

      delete counter_ptr_;
    }
    // else do nothing
  }

  void inc_ref() noexcept
  {
    assert(ptr_);
    assert(counter_ptr_);
    ++*counter_ptr_;
  }

private:
  using atomic_type = std::atomic<counter_type>;

  atomic_type* counter_ptr_{};

  typename ::detail::remove_array<T>::type* ptr_;
};

#endif // LIGHTPTR_HPP
