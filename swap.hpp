#ifndef GENERIC_SWAP_HPP
# define GENERIC_SWAP_HPP

#include <cstdint>

#if defined(_MSC_VER)
# include <stdlib.h>
#endif // _MSC_VER

namespace generic
{

#if defined(__GNUC__)

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{} &&
  (sizeof(::std::int64_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return __builtin_bswap64(v);
}

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{} &&
  (sizeof(::std::int32_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return __builtin_bswap32(v);
}

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{} &&
  (sizeof(::std::int16_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return __builtin_bswap16(v);
}

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{} &&
  (sizeof(::std::int8_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return v;
}

#elif defined(_MSC_VER)

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{} &&
  (sizeof(__int64) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return _byteswap_uint64(i);
}

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{} &&
  (sizeof(unsigned long) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return _byteswap_ulong(v);
}

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{} &&
  (sizeof(unsigned short) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return _byteswap_ushort(v)
}

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{} &&
  (sizeof(::std::int8_t) == sizeof(T)),
  T
>
swap(T const v) noexcept
{
  return v;
}

#endif

#if defined(__BYTE_ORDER) && __BYTE_ORDER == __BIG_ENDIAN || \
    defined(__BIG_ENDIAN__) || \
    defined(__ARMEB__) || \
    defined(__THUMBEB__) || \
    defined(__AARCH64EB__) || \
    defined(_MIBSEB) || defined(__MIBSEB) || defined(__MIBSEB__)

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{},
  T
>
swap_if_le(T const i) noexcept
{
  return i;
}

template <typename T>
inline ::std::enable_if_t<
  ::std::is_integral<T>{},
  T
>
swap_if_be(T const i) noexcept
{
  return swap(i);
}

#elif defined(__BYTE_ORDER) && __BYTE_ORDER == __LITTLE_ENDIAN || \
    defined(__LITTLE_ENDIAN__) || \
    defined(__ARMEL__) || \
    defined(__THUMBEL__) || \
    defined(__AARCH64EL__) || \
    defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__)

template <typename T>
constexpr inline ::std::enable_if_t<
  ::std::is_integral<T>{},
  T
>
swap_if_be(T const i) noexcept
{
  return i;
}

template <typename T>
inline ::std::enable_if_t<
  ::std::is_integral<T>{},
  T
>
swap_if_le(T const i) noexcept
{
  return swap(i);
}

#else
#error "I don't know what architecture this is!"
#endif

}

#endif // GENERIC_SWAP_HPP
