#pragma once
#ifndef LIGHTPTR_HPP
# define LIGHTPTR_HPP

#include <cassert>

#include <atomic>

#include <memory>

#include <type_traits>

namespace detail
{
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

  using counter_type = std::size_t;

  using deleter_type = void (*)(void*);

  using element_type = typename remove_array<T>::type;

  light_ptr() = default;

  explicit light_ptr(element_type* const p,
    deleter_type const d = default_deleter<T>) :
    counter_ptr_(new atomic_type(counter_type(1))),
    ptr_(p),
    deleter_(d)
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

        deleter_ = other.deleter_;

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
    deleter_ = rhs.deleter_;

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

  typename ::detail::ref_type<element_type>::type
  operator*() const noexcept
  {
    return *static_cast<T*>(static_cast<void*>(ptr_));
  }

  T* operator->() const noexcept
  {
    return static_cast<T*>(static_cast<void*>(ptr_));
  }

  T* get() const noexcept
  {
    return counter_ptr_ ? static_cast<T*>(static_cast<void*>(ptr_)) : nullptr;
  }

  void reset(element_type* const p = nullptr,
    deleter_type const d = default_deleter<T>)
  {
    dec_ref();

    if (p)
    {
      counter_ptr_ = new atomic_type(counter_type(1));

      ptr_ = p;

      deleter_ = d;
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
    if (counter_ptr_ && *counter_ptr_ && !--*counter_ptr_)
    {
      delete counter_ptr_;

      deleter_(ptr_);
    }
    // else do nothing
  }

  void inc_ref() noexcept
  {
    assert(ptr_);
    assert(counter_ptr_);
    ++*counter_ptr_;
  }

  static void default_deleter(void* const p)
  {
    std::default_delete<T>()(static_cast<element_type*>(p));
  }

private:
  using atomic_type = std::atomic<counter_type>;

  atomic_type* counter_ptr_{};

  element_type* ptr_;

  deleter_type deleter_;
};

#endif // LIGHTPTR_HPP
