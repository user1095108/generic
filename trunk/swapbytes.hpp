#ifndef SWAPBYTES_HPP
# define SWAPBYTES_HPP

#include <algorithm>

#include <cassert>

#include <cstdint>

//////////////////////////////////////////////////////////////////////////////
static inline void swap_bytes(std::int64_t& u)
{
  u = (u >> 56) |
    ((u << 40) & 0x00ff000000000000ull) |
    ((u << 24) & 0x0000ff0000000000ull) |
    ((u << 8)  & 0x000000ff00000000ull) |
    ((u >> 8)  & 0x00000000ff000000ull) |
    ((u >> 24) & 0x0000000000ff0000ull) |
    ((u >> 40) & 0x000000000000ff00ull) |
    (u << 56);
}

//////////////////////////////////////////////////////////////////////////////
static inline void swap_bytes(std::uint64_t& u)
{
  u = (u >> 56) |
    ((u << 40) & 0x00ff000000000000ull) |
    ((u << 24) & 0x0000ff0000000000ull) |
    ((u << 8)  & 0x000000ff00000000ull) |
    ((u >> 8)  & 0x00000000ff000000ull) |
    ((u >> 24) & 0x0000000000ff0000ull) |
    ((u >> 40) & 0x000000000000ff00ull) |
    (u << 56);
}

//////////////////////////////////////////////////////////////////////////////
static inline void swap_bytes(std::int32_t& u)
{
  u = (u >> 24) |
    ((u << 8) & 0x00ff0000u) |
    ((u >> 8) & 0x0000ff00u) |
    (u << 24);
}

//////////////////////////////////////////////////////////////////////////////
static inline void swap_bytes(std::uint32_t& u)
{
  u = (u >> 24) |
    ((u << 8) & 0x00ff0000u) |
    ((u >> 8) & 0x0000ff00u) |
    (u << 24);
}

//////////////////////////////////////////////////////////////////////////////
static inline void swap_bytes(std::int16_t& u)
{
  u = (u >> 8) |
    (u << 8);
}

//////////////////////////////////////////////////////////////////////////////
static inline void swap_bytes(std::uint16_t& u)
{
  u = (u >> 8) |
    (u << 8);
}

//////////////////////////////////////////////////////////////////////////////
static inline void swap_bytes(std::int8_t&)
{
  assert(0);
}

//////////////////////////////////////////////////////////////////////////////
static inline void swap_bytes(std::uint8_t&)
{
  assert(0);
}

//////////////////////////////////////////////////////////////////////////////
template <class T>
static inline void swap_bytes(T& u)
{
  std::uint8_t* v(reinterpret_cast<std::uint8_t*>(&u));

  for (std::uint8_t i(0); i != sizeof(u) >> 1; ++i)
  {
    std::swap(v[i], v[sizeof(u) - 1]);
  }
}

#endif // SWAPBYTES_HPP
