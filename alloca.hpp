#ifndef GENERIC_ALLOCA_HPP
# define GENERIC_ALLOCA_HPP
# pragma once

#include <cstddef>

#if defined(__GNUC__)
# include <alloca.h>
#elif defined(_MSC_VER)
# include <malloc.h>
#endif //

namespace generic
{

template <typename T = char, typename F>
constexpr inline void salloc(::std::size_t const N, F&& f) noexcept(
  noexcept(f(nullptr))
)
{
#if defined(__GNUC__)
  f(static_cast<T*>(alloca(N * sizeof(T))));
#elif defined(_MSC_VER)
  f(static_cast<T*>(_alloca(N * sizeof(T))));
#else
  alignas(::std::max_align_t) char p[N * sizeof(T)];
  f(reinterpret_cast<T*>(p));
#endif //
}

}

#endif // GENERIC_ALLOCA_HPP
