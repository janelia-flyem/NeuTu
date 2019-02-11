#ifndef ZUTILS_H
#define ZUTILS_H

#include "qt/core/zexception.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <memory>
#include <iostream>
#include <type_traits>
#include <QtGlobal> // not needed for qt >= 5.7
#include <QDebug>

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
#if 0 //for compatibility for gcc 4.8*
  static_assert(std::is_trivially_copyable<Dest>::value,
                "non-trivially-copyable bit_cast is undefined");
  static_assert(std::is_trivially_copyable<Source>::value,
                "non-trivially-copyable bit_cast is undefined");
#endif
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

class _KeyLess
{
public:
  template<typename KeyType>
  static KeyType getKey(const KeyType& k)
  {
    return k;
  }

  template<typename KeyType, typename ValueType>
  static KeyType getKey(const std::pair<const KeyType, ValueType>& p)
  {
    return p.first;
  }

  template<typename L, typename R>
  bool operator()(const L& l, const R& r) const
  {
#ifdef _DEBUG_2
    qDebug() << "Key compare: " << getKey(l) << " " << getKey(r);
#endif

    return getKey(l) < getKey(r);
  }
};

class _KeyEqual
{
public:
  template<typename KeyType>
  static KeyType getKey(const KeyType& k)
  {
    return k;
  }

  template<typename KeyType, typename ValueType>
  static KeyType getKey(const std::pair<const KeyType, ValueType>& p)
  {
    return p.first;
  }

  template<typename L, typename R>
  bool operator()(const L& l, const R& r) const
  {
#ifdef _DEBUG_2
    qDebug() << "Key compare: " << getKey(l) << " " << getKey(r);
#endif

    return getKey(l) == getKey(r);
  }
};

#if QT_VERSION < QT_VERSION_CHECK(5, 7, 0)
template <typename... Args>
struct QNonConstOverload
{
  template <typename R, typename T>
  constexpr auto operator()(R (T::*ptr)(Args...)) const noexcept -> decltype(ptr)
  { return ptr; }

  template <typename R, typename T>
  static constexpr auto of(R (T::*ptr)(Args...)) noexcept -> decltype(ptr)
  { return ptr; }
};

template <typename... Args>
struct QConstOverload
{
  template <typename R, typename T>
  constexpr auto operator()(R (T::*ptr)(Args...) const) const noexcept -> decltype(ptr)
  { return ptr; }

  template <typename R, typename T>
  static constexpr auto of(R (T::*ptr)(Args...) const) noexcept -> decltype(ptr)
  { return ptr; }
};

template <typename... Args>
struct QOverload : QConstOverload<Args...>, QNonConstOverload<Args...>
{
  using QConstOverload<Args...>::of;
  using QConstOverload<Args...>::operator();
  using QNonConstOverload<Args...>::of;
  using QNonConstOverload<Args...>::operator();

  template <typename R>
  constexpr auto operator()(R (*ptr)(Args...)) const noexcept -> decltype(ptr)
  { return ptr; }

  template <typename R>
  static constexpr auto of(R (*ptr)(Args...)) noexcept -> decltype(ptr)
  { return ptr; }
};
#endif

// for c++11
#if __cplusplus == 201103L
namespace std {

template<class T> struct _Unique_if {
  typedef unique_ptr<T> _Single_object;
};

template<class T> struct _Unique_if<T[]> {
  typedef unique_ptr<T[]> _Unknown_bound;
};

template<class T, size_t N> struct _Unique_if<T[N]> {
  typedef void _Known_bound;
};

template<class T, class... Args>
typename _Unique_if<T>::_Single_object
make_unique(Args&&... args) {
  return unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template<class T>
typename _Unique_if<T>::_Unknown_bound
make_unique(size_t n) {
  typedef typename remove_extent<T>::type U;
  return unique_ptr<T>(new U[n]());
}

template<class T, class... Args>
typename _Unique_if<T>::_Known_bound
make_unique(Args&&...) = delete;

}
#endif

#endif // ZUTILS_H
