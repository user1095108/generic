#pragma once
#ifndef LIGHTPTR_HPP
# define LIGHTPTR_HPP

#include <cassert>

#include <atomic>

#include <memory>

#include <utility>

#include <type_traits>

namespace generic
{

namespace detail
{
  using counter_type = unsigned;

  using atomic_type = ::std::atomic<counter_type>;

  template <typename T>
  using deleter_type = void (*)(T*);

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
  template <typename U, typename V>
  struct deletion_type
  {
    using type = V;
  };

  template <typename U, typename V>
  struct deletion_type<U[], V>
  {
    using type = V[];
  };

  template <typename U, typename V, ::std::size_t N>
  struct deletion_type<U[N], V>
  {
    using type = V[];
  };

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

  template <typename U, ::std::size_t N>
  struct remove_array<U[N]>
  {
    using type = U;
  };

  using element_type = typename remove_array<T>::type;

  using deleter_type = detail::deleter_type<element_type>;

  struct counter_base
  {
    using invoker_type = void (*)(counter_base*, element_type*);

    explicit counter_base(detail::counter_type c,
      invoker_type invoker) noexcept :
      counter_(c),
      invoker_(invoker)
    {
    }

    template <typename U>
    typename ::std::enable_if<!::std::is_void<U>{}>::type
    dec_ref(U* const ptr)
    {
      if (detail::counter_type(1) ==
        counter_.fetch_sub(detail::counter_type(1),
          ::std::memory_order_relaxed))
      {
        typedef char type_must_be_complete[sizeof(U) ? 1 : -1];
        (void)sizeof(type_must_be_complete);
        invoker_(this, ptr);
      }
      // else do nothing
    }

    template <typename U>
    typename ::std::enable_if<::std::is_void<U>{}>::type
    dec_ref(U* const ptr)
    {
      if (detail::counter_type(1) ==
        counter_.fetch_sub(detail::counter_type(1),
          ::std::memory_order_relaxed))
      {
        invoker_(this, ptr);
      }
      // else do nothing
    }

    void inc_ref() noexcept
    {
      //assert(counter_ptr);
      counter_.fetch_add(detail::counter_type(1),
        ::std::memory_order_relaxed);
    }

    detail::atomic_type counter_{};

    invoker_type invoker_;
  };

  template <typename U>
  struct counter : counter_base
  {
    explicit counter(detail::counter_type const c, U&& d) noexcept :
      counter_base(c, invoker),
      d_(::std::forward<U>(d))
    {
    }

  private:
    static void invoker(counter_base* const ptr, element_type* const e)
    {
      auto const c(static_cast<counter<U>*>(ptr));

      typename ::std::decay<U>::type const d(::std::move(c->d_));

      delete c;

      d(e);
    }

  private:
    typename ::std::decay<U>::type d_;
  };

  light_ptr() = default;

  template <typename U>
  explicit light_ptr(U* const p, deleter_type const d = default_deleter<U>)
  {
    reset(p, d);
  }

  ~light_ptr()
  {
    if (counter_)
    {
      counter_->dec_ref(ptr_);
    }
    // else do nothing
  }

  light_ptr(light_ptr const& other) { *this = other; }

  light_ptr(light_ptr&& other) noexcept { *this = ::std::move(other); }

  light_ptr& operator=(light_ptr const& rhs)
  {
    if (*this != rhs)
    {
      if (counter_)
      {
        counter_->dec_ref(ptr_);
      }
      // else do nothing

      counter_ = rhs.counter_;
      ptr_ = rhs.ptr_;

      if (counter_)
      {
        counter_->inc_ref();
      }
      // else do nothing
    }
    // else do nothing

    return *this;
  }

  light_ptr& operator=(light_ptr&& rhs) noexcept
  {
    counter_ = rhs.counter_;
    ptr_ = rhs.ptr_;

    rhs.counter_ = nullptr;
    rhs.ptr_ = nullptr;

    return *this;
  }

  light_ptr& operator=(::std::nullptr_t const) noexcept { reset(); }

  bool operator<(light_ptr const& rhs) const noexcept
  {
    return get() < rhs.get();
  }

  bool operator==(light_ptr const& rhs) const noexcept
  {
    return counter_ == rhs.counter_;
  }

  bool operator!=(light_ptr const& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  bool operator==(::std::nullptr_t const) const noexcept
  {
    return !ptr_;
  }

  bool operator!=(::std::nullptr_t const) const noexcept
  {
    return ptr_;
  }

  explicit operator bool() const noexcept { return ptr_; }

  typename detail::ref_type<T>::type
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

  void reset(::std::nullptr_t const)
  {
    if (counter_)
    {
      counter_->dec_ref(ptr_);

      counter_ = {};
    }
    // else do nothing

    ptr_ = {};
  }

  template <typename U>
  static void default_deleter(element_type* const p)
  {
    ::std::default_delete<typename deletion_type<T, U>::type>()(
      static_cast<U*>(p));
  }

  template <typename U, typename D>
  void reset(U* const p, D&& d = default_deleter<U>)
  {
    if (counter_)
    {
      counter_->dec_ref(ptr_);
    }
    // else do nothing

    counter_ = new counter<D>(detail::counter_type(1), ::std::forward<D>(d));

    ptr_ = p;
  }

  void swap(light_ptr& other) noexcept
  {
    ::std::swap(counter_, other.counter_);
    ::std::swap(ptr_, other.ptr_);
  }

  bool unique() const noexcept
  {
    return detail::counter_type(1) == use_count();
  }

  detail::counter_type use_count() const noexcept
  {
    return counter_ ?
      counter_->counter_ptr_->load(::std::memory_order_relaxed) :
      detail::counter_type{};
  }

private:
  counter_base* counter_{};

  element_type* ptr_{};
};

template<class T, class ...Args>
inline light_ptr<T> make_light(Args&& ...args)
{
  return light_ptr<T>(new T(::std::forward<Args>(args)...));
}

}

namespace std
{
  template <typename T>
  struct hash<::generic::light_ptr<T> >
  {
    size_t operator()(::generic::light_ptr<T> const& l) const noexcept
    {
      return hash<typename ::generic::light_ptr<T>::element_type*>()(l.get());
    }
  };
}

#endif // LIGHTPTR_HPP
