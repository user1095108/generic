#ifndef GENERIC_ALLOCA_HPP
# define GENERIC_ALLOCA_HPP
# pragma once

#ifndef PRIMITIVE_CAT
# define PRIMITIVE_CAT(x, y) x ## y
#endif // PRIMITIVE_CAT

#ifndef CAT
# define CAT(x, y) PRIMITIVE_CAT(x, y)
#endif // CAT

#if defined(__GNUC__)
# include <alloca.h>
# define ALLOCA(N) alloca(N)
#elif defined(_MSC_VER)
# include <malloc.h>
# define ALLOCA(N) _alloca(N)
#else
# error "cannot find alloca"
#endif //

#endif // GENERIC_ALLOCA_HPP
