#ifndef UTILITY_HPP
# define UTILITY_HPP

#include <cstddef>

namespace generic
{

template <::std::size_t...> struct indices
{
};

template <::std::size_t M, ::std::size_t ...Is>
struct make_indices
{
};

               0
              1 2
            3 4 5 6

}

#endif // UTILITY_HPP
