#ifndef GENERIC_ALLOCA_HPP
# define GENERIC_ALLOCA_HPP
# pragma once

#include <cstddef> // ::std::size_t

#if defined(_MSC_VER)
# include <malloc.h>
#else
# include <alloca.h>
#endif

#if defined(_MSC_VER)
# define ALLOCA _malloca
#else
# define ALLOCA alloca
#endif

#endif // GENERIC_ALLOCA_HPP
