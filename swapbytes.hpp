#ifndef SWAPBYTES_HPP
# define SWAPBYTES_HPP

#include <cassert>

#include <cstdint>

#ifdef __GNUC__

//////////////////////////////////////////////////////////////////////////////
static inline std::int64_t swap_bytes(std::int64_t u)
{
  return __builtin_bswap64(u)
}

//////////////////////////////////////////////////////////////////////////////
static inline std::uint64_t swap_bytes(std::uint64_t u)
{
  return __builtin_bswap64(u)
}

//////////////////////////////////////////////////////////////////////////////
static inline std::int32_t swap_bytes(std::int32_t u)
{
  return __builtin_bswap32(u)
}

//////////////////////////////////////////////////////////////////////////////
static inline std::uint32_t swap_bytes(std::uint32_t u)
{
  return __builtin_bswap32(u)
}

//////////////////////////////////////////////////////////////////////////////
static inline std::int16_t swap_bytes(std::int16_t const u)
{
  return __builtin_bswap16(u)
}

//////////////////////////////////////////////////////////////////////////////
static inline std::uint16_t swap_bytes(std::uint16_t const u)
{
  return __builtin_bswap16(u);
}

#else

//////////////////////////////////////////////////////////////////////////////
static inline std::int64_t swap_bytes(std::int64_t u)
{
  return (u >> 56) |
    ((u << 40) & 0x00ff000000000000ull) |
    ((u << 24) & 0x0000ff0000000000ull) |
    ((u << 8)  & 0x000000ff00000000ull) |
    ((u >> 8)  & 0x00000000ff000000ull) |
    ((u >> 24) & 0x0000000000ff0000ull) |
    ((u >> 40) & 0x000000000000ff00ull) |
    (u << 56);
}

//////////////////////////////////////////////////////////////////////////////
static inline std::uint64_t swap_bytes(std::uint64_t u)
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
static inline std::int32_t swap_bytes(std::int32_t u)
{
  return (u >> 24) |
    ((u << 8) & 0x00ff0000u) |
    ((u >> 8) & 0x0000ff00u) |
    (u << 24);
}

//////////////////////////////////////////////////////////////////////////////
static inline std::uint32_t swap_bytes(std::uint32_t u)
{
  return (u >> 24) |
    ((u << 8) & 0x00ff0000u) |
    ((u >> 8) & 0x0000ff00u) |
    (u << 24);
}

//////////////////////////////////////////////////////////////////////////////
static inline std::uint16_t swap_bytes(std::int16_t u)
{
  return (u >> 8) | (u << 8);
}

//////////////////////////////////////////////////////////////////////////////
static inline std::uint16_t swap_bytes(std::uint16_t u)
{
  return (u >> 8) | (u << 8);
}

#endif // __GNUC__

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

#endif // SWAPBYTES_HPP
