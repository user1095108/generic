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

template <typename F, typename T = char>
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

}

#endif // GNR_ALLOCA_HPP
