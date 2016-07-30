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

template <typename F>
constexpr inline void alloc_stack(::std::size_t const N, F&& f) noexcept(
  noexcept(f(nullptr))
)
{
#if defined(__GNUC__)
  f(alloca(N));
#elif defined(_MSC_VER)
  f(_alloca(N));
#else
  alignas(::std::max_align_t) char p[N];
  f(p);
#endif //
}

}

#endif // GENERIC_ALLOCA_HPP
