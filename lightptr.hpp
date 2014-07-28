#ifndef LIGHTPTR_HPP
# define LIGHTPTR_HPP
# pragma once

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
class light_ptr
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
  struct is_array_type : ::std::false_type { };

  template <typename U>
  struct is_array_type<U[]> : ::std::true_type { };

  template <typename U, ::std::size_t N>
  struct is_array_type<U[N]> : ::std::true_type { };

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

  class counter_base
  {
    friend class light_ptr;

    using invoker_type = void (*)(counter_base*, element_type*);

    detail::atomic_type counter_{};

    invoker_type const invoker_;

  protected:
    explicit counter_base(detail::counter_type const c,
      invoker_type const invoker) noexcept :
      counter_(c),
      invoker_(invoker)
    {
    }

  public:
    template <typename U>
    typename ::std::enable_if<!::std::is_void<U>{}>::type
    dec_ref(U* const ptr)
    {
      if (detail::counter_type(1) ==
        counter_.fetch_sub(detail::counter_type(1),
          ::std::memory_order_relaxed))
      {
        using type_must_be_complete = char[sizeof(U) ? 1 : -1];
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
      counter_.fetch_add(detail::counter_type(1),
        ::std::memory_order_relaxed);
    }
  };

  template <typename D>
  class counter : public counter_base
  {
    typename ::std::decay<D>::type const d_;

  public:
    explicit counter(detail::counter_type const c, D&& d) :
      counter_base(c, invoker),
      d_(::std::forward<D>(d))
    {
    }

  private:
    static void invoker(counter_base* const ptr, element_type* const e)
    {
      auto const c(static_cast<counter<D>*>(ptr));

      // invoke deleter on the element
      c->d_(e);

      // delete from a static member function
      delete c;
    }
  };

private:
  template <typename U> friend struct ::std::hash;

  counter_base* counter_{};

  element_type* ptr_{};

public:
  light_ptr() = default;

  template <typename U>
  explicit light_ptr(U* p)
  {
    reset(p);
  }

  template <typename U, typename D>
  explicit light_ptr(U* p, D&& d)
  {
    reset(p, ::std::forward<D>(d));
  }

  light_ptr(light_ptr const& other) { *this = other; }

  light_ptr(light_ptr&& other) noexcept { *this = ::std::move(other); }

  ~light_ptr()
  {
    if (counter_)
    {
      counter_->dec_ref(ptr_);
    }
    // else do nothing
  }

  light_ptr& operator=(light_ptr const& rhs)
  {
    if (*this != rhs)
    {
      if (counter_)
      {
        counter_->dec_ref(ptr_);
      }
      // else do nothing

      if ((counter_ = rhs.counter_))
      {
        counter_->inc_ref();
      }
      // else do nothing

      ptr_ = rhs.ptr_;
    }
    // else do nothing

    return *this;
  }

  light_ptr& operator=(light_ptr&& rhs) noexcept
  {
    counter_ = rhs.counter_;
    rhs.counter_ = nullptr;

    ptr_ = rhs.ptr_;
    rhs.ptr_ = nullptr;

    return *this;
  }

  light_ptr& operator=(::std::nullptr_t const) noexcept { reset(); }

  bool operator<(light_ptr const& rhs) const noexcept
  {
    return counter_ < rhs.counter_;
  }

  bool operator==(light_ptr const& rhs) const noexcept
  {
    return counter_ == rhs.counter_;
  }

  bool operator!=(light_ptr const& rhs) const noexcept
  {
    return !operator==(rhs);
  }

  bool operator==(::std::nullptr_t const) const noexcept { return !ptr_; }

  bool operator!=(::std::nullptr_t const) const noexcept { return ptr_; }

  explicit operator bool() const noexcept { return ptr_; }

  typename detail::ref_type<T>::type
  operator*() const noexcept
  {
    return *reinterpret_cast<T*>(ptr_);
  }

  T* operator->() const noexcept { return reinterpret_cast<T*>(ptr_); }

  template <typename U = T, typename =
    typename ::std::enable_if<is_array_type<U>{}>::type>
  typename detail::ref_type<element_type>::type operator[](
    ::std::size_t const i) const noexcept
  {
    return ptr_[i];
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
  void reset(U* const p)
  {
    reset(p, [](element_type* const p) {
      ::std::default_delete<typename deletion_type<T, U>::type>()(
        static_cast<U*>(p));
    });
  }

  template <typename U, typename D>
  void reset(U* const p, D&& d)
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
      counter_->counter_.load(::std::memory_order_relaxed) :
      detail::counter_type{};
  }
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
      return hash<typename ::generic::light_ptr<T>::element_type*>()(
        l.counter_);
    }
  };
}

#endif // LIGHTPTR_HPP
