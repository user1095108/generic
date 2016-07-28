#ifndef GENERIC_ALLOCA_HPP
# define GENERIC_ALLOCA_HPP
# pragma once

#if defined(_MSC_VER)
# include <malloc.h>
# define ALLOCA _alloca
#else
# include <alloca.h>
# define ALLOCA alloca
#endif //

#endif // GENERIC_ALLOCA_HPP
