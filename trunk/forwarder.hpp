#include <cassert>

#include <cstddef>

#include <cstdint>

#include <type_traits>

#include <utility>

#include <new>

namespace generic
{

template<typename F>
class forwarder;

template<typename R, typename ...A>
class forwarder<R (A...)>
{
public:
  forwarder() = default;

  template<typename T>
  forwarder(T&& f) noexcept :
    stub_(&handler<T>::invoke)
  {
    static_assert(sizeof(T) <= sizeof(store_), "functor too large");
    static_assert(::std::is_trivially_destructible<T>::value,
      "functor not trivially destructible");
    new (&store_) handler<T>(::std::forward<T>(f));
  }

  forwarder& operator=(forwarder const&) = delete;

  template <
    typename T,
    typename = typename ::std::enable_if<
      !::std::is_same<forwarder, typename ::std::decay<T>::type>{}
    >::type
  >
  forwarder& operator=(T&& f) noexcept
  {
    static_assert(sizeof(T) <= sizeof(store_), "functor too large");
    static_assert(::std::is_trivially_destructible<T>::value,
      "functor not trivially destructible");
    stub_ = &handler<T>::invoke;
    new (&store_) handler<T>(::std::forward<T>(f));
  }

  R operator() (A... args)
  {
    //assert(stub_);
    return stub_(&store_, ::std::forward<A>(args)...);
  }

private:
  template<typename T>
  struct handler
  {
    T functor_;

    handler(T&& f) noexcept : functor_(::std::forward<T>(f)) { }

    static R invoke(void* ptr, A&&... args)
    {
      return static_cast<handler<T>*>(ptr)->functor_(::std::forward<A>(args)...);
    }
  };

#if defined(__clang__)
  using max_align_type = long double;
#elif defined(__GNUC__)
  using max_align_type = ::max_align_t;
#else
  using max_align_type = ::std::max_align_t;
#endif

  alignas(max_align_type) ::std::uintptr_t store_;

  R (*stub_)(void*, A&&...){};
};

}
