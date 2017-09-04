#ifndef GNR_ALLOCA_HPP
# define GNR_ALLOCA_HPP
# pragma once

#include <cstddef>

#include <cstdint>

#if defined(__linux__)
# include <alloca.h>
#elif defined(_WIN32)
# include <malloc.h>
#endif //

#if (__cplusplus > 201402L)
#include <cstring>

#include <string_view>
#endif // __cplusplus

namespace gnr
{

template <std::size_t N, typename T = char, typename F>
inline void salloc(F&& f) noexcept(noexcept(f(nullptr)))
{
#if defined(__linux__)
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
#if defined(__linux__)
  f(static_cast<T*>(alloca(N * sizeof(T))));
#elif defined(_WIN32)
  f(static_cast<T*>(_alloca(N * sizeof(T))));
#else
  alignas(std::max_align_t) std::uint8_t p[N * sizeof(T)];

  f(reinterpret_cast<T*>(p));
#endif //
}

#if (__cplusplus > 201402L)

template <typename F>
inline void c_str(std::string_view const& sv, F&& f) noexcept(
  noexcept(f(nullptr))
)
{
  salloc(sv.size() + 1,
    [&](auto const p) noexcept(noexcept(f(nullptr)))
    {
      std::memcpy(p, sv.data(), sv.size());
      p[sv.size()] = '\0';

      f(p);
    }
  );
}

#endif // __cplusplus

}

#endif // GNR_ALLOCA_HPP
