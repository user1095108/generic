#ifndef GENERIC_IS_DEFINED_HPP
# define GENERIC_IS_DEFINED_HPP
# pragma once

#define IS_DEFINED(x)\
  #if defined(x)     \
    true             \
  #else              \
    false            \

#endif // GENERIC_IS_DEFINED_HPP
