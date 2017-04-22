#ifndef GNR_SWAP_HPP
# define GNR_SWAP_HPP
# pragma once

#include <cstdint>

#include <cstring>

#include <type_traits>

#if defined(_MSC_VER)
# include <stdlib.h>
#endif // _MSC_VER

namespace gnr
{

template <class Dst, class Src>
inline Dst bit_cast(Src const src) noexcept
{
  Dst dst;

  std::memcpy(&dst, &src, sizeof(dst));

  return dst;
}

template <class Dst, class Src>
inline Dst bit_cast(Src* const src) noexcept
{
  Dst dst;

  std::memcpy(&dst, src, sizeof(dst));

  return dst;
}

#if defined(__GNUC__)

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{} &&
  (sizeof(std::int64_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return __builtin_bswap64(v);
}

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{} &&
  (sizeof(std::int32_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return __builtin_bswap32(v);
}

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{} &&
  (sizeof(std::int16_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return __builtin_bswap16(v);
}

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{} &&
  (sizeof(std::int8_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return v;
}

#elif defined(_MSC_VER)

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{} &&
  (sizeof(__int64) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return _byteswap_uint64(i);
}

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{} &&
  (sizeof(unsigned long) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return _byteswap_ulong(v);
}

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{} &&
  (sizeof(unsigned short) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return _byteswap_ushort(v)
}

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{} &&
  (sizeof(std::int8_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return v;
}

#else

namespace
{

template<class T, std::size_t ...I>
constexpr inline T swap_impl(T v, std::index_sequence<I...> const) noexcept
{
  return (
    (
      ((v >> (I * std::numeric_limits<char>::digits)) & std::uint8_t(~0)) <<
      ((sizeof(T) - 1 - I) * std::numeric_limits<char>::digits)
    ) | ...
  );
};

}

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{},
  T
>
swap(T const v) noexcept
{
  return swap_impl<T>(v, std::make_index_sequence<sizeof(T)>{});
}

#endif

#if (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) || \
  (defined(__BIG_ENDIAN__) && !defined(__LITTLE_ENDIAN__)) || \
  (defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)) || \
  defined(__ARMEB__) || \
  defined(__THUMBEB__) || \
  defined(__AARCH64EB__) || \
  defined(_MIPSEB) || \
  defined(__MIPSEB) || \
  defined(__MIPSEB__)
template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{},
  T
>
from_be(T const i) noexcept
{
  return i;
}

template <typename T>
inline std::enable_if_t<
  std::is_integral<T>{},
  T
>
from_le(T const i) noexcept
{
  return swap(i);
}

#elif (defined(__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) || \
  (defined(__LITTLE_ENDIAN__) && !defined(__BIG_ENDIAN__)) || \
  (defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)) || \
  defined(__ARMEL__) || \
  defined(__THUMBEL__) || \
  defined(__AARCH64EL__) || \
  defined(_MIPSEL) || \
  defined(__MIPSEL) || \
  defined(__MIPSEL__)

template <typename T>
constexpr inline std::enable_if_t<
  std::is_integral<T>{},
  T
>
from_le(T const i) noexcept
{
  return i;
}

template <typename T>
inline std::enable_if_t<
  std::is_integral<T>{},
  T
>
from_be(T const i) noexcept
{
  return swap(i);
}

#else
#error "I don't know what architecture this is!"
#endif

}

#endif // GNR_SWAP_HPP
