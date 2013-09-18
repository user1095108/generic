#pragma once
#ifndef STACKALLOCATOR_HPP
# define STACKALLOCATOR_HPP

#include <cassert>

#include <cstddef>

#include <functional>

#include <new>

#include <utility>

template <std::size_t N>
class stack_store
{
public:
  stack_store() = default;

  stack_store(stack_store const&) = delete;

  stack_store& operator=(stack_store const&) = delete;

  char* allocate(std::size_t n)
  {
    assert(pointer_in_buffer(ptr_) &&
      "stack_allocator has outlived stack_store");

    n = align(n);

    if (buf_ + N >= ptr_ + n)
    {
      auto r(ptr_);

      ptr_ += n;

      return r;
    }
    else
    {
      return static_cast<char*>(::operator new(n));
    }
  }

  void deallocate(char* p, std::size_t n) noexcept
  {
    assert(pointer_in_buffer(ptr_) &&
      "stack_allocator has outlived stack_store");

    if (pointer_in_buffer(p))
    {
      n = align(n);

      if (p + n == ptr_)
      {
        ptr_ = p;
      }
      // else do nothing
    }
    else
    {
      ::operator delete(p);
    }
  }

  void reset() noexcept { ptr_ = buf_; }

  static constexpr ::std::size_t size() noexcept { return N; }

  ::std::size_t used() const { return ::std::size_t(ptr_ - buf_); }

private:
  static constexpr ::std::size_t align(std::size_t const n) noexcept
  {
    return (n + alignment - 1) & -alignment;
  }

  bool pointer_in_buffer(char* const p) noexcept
  {
    return (buf_ <= p) && (p <= buf_ + N);
  }

private:
  static constexpr auto const alignment = alignof(::max_align_t);

  char* ptr_{buf_};

  alignas(::max_align_t) char buf_[N];
};

template <class T, std::size_t N>
class stack_allocator
{
public:
  using store_type = stack_store<N>;

  using size_type = ::std::size_t;

  using difference_type = ::std::ptrdiff_t;

  using pointer = T*;
  using const_pointer = T const*;

  using reference = T&;
  using const_reference = T const&;

  using value_type = T;

  template <class U> struct rebind { using other = stack_allocator<U, N>; };

  stack_allocator() = default;

  stack_allocator(stack_store<N>& s) noexcept : store_(&s) { }

  template <class U>
  stack_allocator(stack_allocator<U, N> const& other) noexcept :
    store_(other.store_)
  {
  }

  stack_allocator& operator=(stack_allocator const&) = delete;

  T* allocate(::std::size_t const n)
  {
    return static_cast<T*>(static_cast<void*>(
      store_->allocate(n * sizeof(T))));
  }

  void deallocate(T* const p, ::std::size_t const n) noexcept
  {
    store_->deallocate(static_cast<char*>(static_cast<void*>(p)),
      n * sizeof(T));
  }

  template <class U, class ...A>
  void construct(U* p, A&& ...args)
  {
    new (p) U(::std::forward<A>(args)...);
  }

  template <class U>
  void destroy(U* const p)
  {
    p->~U();
  }

  template <class U, std::size_t M>
  inline bool operator==(stack_allocator<U, M> const& rhs) const noexcept
  {
    return store_ == rhs.store_;
  }

  template <class U, std::size_t M>
  inline bool operator!=(stack_allocator<U, M> const& rhs) const noexcept
  {
    return !(*this == rhs);
  }

private:
  template <class U, std::size_t M> friend class stack_allocator;

  store_type* store_{};
};

namespace std
{
  // string
  template<class CharT> class char_traits;

  template<class CharT, class Traits, class Allocator> class basic_string;

  // unordered_map
  template<class Key, class T, class Hash, class Pred, class Alloc>
    class unordered_map;

  // vector
  template <class T, class Alloc> class vector;
}

using stack_string = ::std::basic_string<char, ::std::char_traits<char>,
  stack_allocator<char, 128> >;

template <class Key, class T, class Hash = ::std::hash<Key>,
  class Pred = ::std::equal_to<Key> >
using stack_unordered_map = ::std::unordered_map<Key, T, Hash, Pred,
  stack_allocator<::std::pair<Key const, T>, 256> >;

template <typename T>
using stack_vector = ::std::vector<T, stack_allocator<T, 256> >;

#endif // STACKALLOCATOR_HPP
