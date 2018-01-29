#ifndef GNR_ALLOCA_HPP
# define GNR_ALLOCA_HPP
# pragma once

#include <cstddef>

#include <cstdint>

#if defined(__linux__)
# include <alloca.h>
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) ||\
  defined(__bsdi__) || defined(__DragonFly__)
# include <stdlib.h>
#elif defined(_WIN32)
# include <malloc.h>
#endif //

#if defined(__cplusplus) && (__cplusplus > 201402L)
#include <cstring>

#include <string_view>
#endif // __cplusplus

namespace gnr
{

template <std::size_t N, typename T = char, typename F>
inline void salloc(F&& f) noexcept(noexcept(f(nullptr)))
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||\
  defined(__NetBSD__) || defined(__bsdi__) || defined(__DragonFly__)
  f(static_cast<T*>(alloca(N * sizeof(T))));
#elif defined(_WIN32)
  f(static_cast<T*>(_alloca(N * sizeof(T))));
#else
  alignas(std::max_align_t) std::uint8_t p[N * sizeof(T)];

  f(reinterpret_cast<T*>(p));
#endif //
}

template <typename T = char, typename F>
inline void salloc(std::size_t const N, F&& f) noexcept(noexcept(f(nullptr)))
{
#if defined(__linux__) || defined(__FreeBSD__) || defined(__OpenBSD__) ||\
  defined(__NetBSD__) || defined(__bsdi__) || defined(__DragonFly__)
  f(static_cast<T*>(alloca(N * sizeof(T))));
#elif defined(_WIN32)
  f(static_cast<T*>(_alloca(N * sizeof(T))));
#else
  alignas(std::max_align_t) std::uint8_t p[N * sizeof(T)];

  f(reinterpret_cast<T*>(p));
#endif //
}

#if defined(__cplusplus) && (__cplusplus > 201402L)

template <typename C, typename F>
inline auto c_str(C&& c, F&& f) noexcept(noexcept(f(nullptr))) ->
  decltype(c.data(), c.size(), void(0))
{
  salloc(c.size() + 1,
    [&c, &f, s = c.size()](char* const p) noexcept(noexcept(f(nullptr)))
    {
      std::memcpy(p, c.data(), s);
      p[s] = '\0';

      f(p);
    }
  );
}

#endif // __cplusplus

}

#endif // GNR_ALLOCA_HPP
