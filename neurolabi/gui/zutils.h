#ifndef ZUTILS_H
#define ZUTILS_H

#include "zexception.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <type_traits>

#ifdef _MSC_VER
#define __warn_unused_result _Check_return_
#else
#define __warn_unused_result  __attribute__((warn_unused_result))
#define __forceinline       inline __attribute__((always_inline))
#endif

template<typename TEnum>
constexpr typename std::underlying_type<TEnum>::type enumToUnderlyingType(TEnum e) noexcept
{
  return static_cast<typename std::underlying_type<TEnum>::type>(e);
}

// define string of enum as:
// template<>
// struct EnumStrings<TEnum>
// {
//   static constexpr const char* const data[] = {
//     "a", "b", "c", ...
//   };
// };
// to enable enumToString for TEnum
template<typename TEnum>
struct EnumStrings
{
  // static constexpr const char* const data[];
};

// crash if enum e is not valid
template<typename TEnum>
constexpr const char* enumToString(TEnum e) noexcept
{
  return EnumStrings<TEnum>::data[static_cast<typename std::underlying_type<TEnum>::type>(e)];
}

// https://chromium.googlesource.com/chromium/src/+/master/base/bit_cast.h
template<class Dest, class Source>
__forceinline Dest bit_cast(const Source& source)
{
  static_assert(sizeof(Dest) == sizeof(Source),
                "bit_cast requires source and destination to be the same size");
  static_assert(std::is_trivially_copyable<Dest>::value,
                "non-trivially-copyable bit_cast is undefined");
  static_assert(std::is_trivially_copyable<Source>::value,
                "non-trivially-copyable bit_cast is undefined");
  Dest dest;
  memcpy(&dest, &source, sizeof(dest));
  return dest;
}

template<typename Type>
inline bool is_aligned(Type* ptr)
{
  return (reinterpret_cast<uintptr_t>(ptr) & (alignof(Type) - 1)) == 0;
}

template<typename Type>
inline bool is_aligned(Type* ptr, size_t a)
{
  return (reinterpret_cast<uintptr_t>(ptr) & (a - 1)) == 0;
}

inline bool hostIsLittleEndian()
{
  int32_t num = 1;
  return *reinterpret_cast<char*>(&num) == 1;
}

template<typename Container>
inline void clearAndDeallocate(Container& c)
{
  Container().swap(c);
}

// effective stl, item 24, Scott Meyers
template<typename MapType, // type of map
  typename KeyArgType,
  typename ValueArgtype>
__forceinline typename MapType::iterator efficientAddOrUpdate(MapType& m, const KeyArgType& k, const ValueArgtype& v)
{
  typename MapType::iterator lb = m.lower_bound(k); // find where k is or should be
  if (lb != m.end() && !(m.key_comp()(k, lb->first))) { // if Ib points to a pair whose key is equiv to k...
    lb->second = v; // update the pair's value
    return lb; // and return an iterator to that pair
  }
  return m.emplace_hint(lb, k, v); // add pair(k, v) to m and return an iterator to the new map element
}

// literal
constexpr size_t operator "" _usize(unsigned long long int n) noexcept { return static_cast<size_t>(n); }
constexpr ptrdiff_t operator "" _isize(unsigned long long int n) noexcept { return static_cast<ptrdiff_t>(n); }
constexpr uint8_t operator "" _u8(unsigned long long int n) noexcept { return static_cast<uint8_t>(n); }
constexpr int8_t operator "" _i8(unsigned long long int n) noexcept { return static_cast<int8_t>(n); }
constexpr uint16_t operator "" _u16(unsigned long long int n) noexcept { return static_cast<uint16_t>(n); }
constexpr int16_t operator "" _i16(unsigned long long int n) noexcept { return static_cast<int16_t>(n); }
constexpr uint32_t operator "" _u32(unsigned long long int n) noexcept { return static_cast<uint32_t>(n); }
constexpr int32_t operator "" _i32(unsigned long long int n) noexcept { return static_cast<int32_t>(n); }
constexpr uint64_t operator "" _u64(unsigned long long int n) noexcept { return static_cast<uint64_t>(n); }
constexpr int64_t operator "" _i64(unsigned long long int n) noexcept { return static_cast<int64_t>(n); }

#endif // ZUTILS_H
