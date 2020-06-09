#ifndef CORE_UTILITIES_H
#define CORE_UTILITIES_H

#include <set>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cmath>
#include <functional>

#include "neulib/core/utilities.h"

#define NT_STR(s) #s
#define NT_XSTR(s) NT_STR(s)

namespace neutu
{

//bool FileExists(const std::string &path);

template<typename T>
void read(std::istream &stream, T &v)
{
  stream.read(reinterpret_cast<char*>(&v), sizeof(T));
}

template<typename T>
void read(std::istream &stream, T &v, size_t n)
{
  stream.read((char*)(&v), sizeof(T) * n);
}

template<typename T>
void write(std::ostream &stream, const T &v)
{
  stream.write((const char*)(&v), sizeof(T));
}

template<typename T>
void write(std::ostream &stream, const T *v, size_t n)
{
  stream.write((const char*)(v), sizeof(T) * n);
}

template <typename T>
std::ostream& operator << (
    typename std::enable_if<std::is_enum<T>::value, std::ostream>::type &stream,
    const T &v)
{
  return stream << static_cast<typename std::underlying_type<T>::type>(v);
}

template<typename T>
void assign(T *out, const T &v);

template<typename T>
std::set<T> intersect(const std::set<T> &s1, const std::set<T> &s2);

template<typename T>
std::set<T> setdiff(const std::set<T> &s1, const std::set<T> &s2);

template <typename T, typename UninaryPred>
void setremoveif(std::set<T> &s, UninaryPred pred)
{
  for (auto iter = s.begin(); iter != s.end();) {
    if (pred(*iter)) {
      s.erase(iter++);
    } else {
      ++iter;
    }
  }
}

// generic solution
template <class T>
int numDigits(T number)
{
  int digits = 0;
  if (number < 0) digits = 1; // remove this line if '-' counts as a digit
  while (number) {
    number /= 10;
    digits++;
  }
  return digits;
}

bool HasEnv(const std::string &name, const std::string &value);
std::string GetEnv(const std::string &name);

std::string GetVersionString();

uint64_t GetTimestamp();

std::string ToString(const void *p);

template<typename T>
std::string ToString(const T &v)
{
  std::ostringstream stream;
  stream << v;
  return stream.str();
}


template<template<class...> class Container, typename T>
std::string ToString(const Container<T> &container, const std::string &delimiter)
{
  std::string result = "";

  typename Container<T>::const_iterator iter = container.begin();
  if (iter != container.end()) {
    result = ToString(*iter);
    ++iter;
  }

  for (; iter != container.end(); ++iter) {
    result += delimiter + ToString(*iter);
  }

  return result;
}

bool UsingLocalHost(const std::string &url);

template<size_t N>
size_t Length(const char (&)[N])
{
    return N - 1;
}

template<size_t N>
size_t Length(const wchar_t (&)[N])
{
    return N - 1;
}

template<template<class...> class Container, typename KeyType, typename ValueType>
ValueType GetValue(const Container<KeyType, ValueType> &m,
                   const KeyType &key, const ValueType &defaultValue)
{
  typename Container<KeyType, ValueType>::const_iterator iter = m.find(key);
  if (iter != m.end()) {
    return iter->second;
  }

  return defaultValue;
}

template<typename T,
         typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
inline bool IsIntegerValue(T v)
{
  return v == std::floor(v);
}

template<typename T>
inline bool WithinOpenRange(const T &x, const T &minv, const T &maxv)
{
  return (x > minv) && (x < maxv);
}

template<typename T>
inline bool WithinCloseRange(const T &x, const T &minv, const T &maxv)
{
  return (x >= minv) && (x <= maxv);
}

template<typename T>
inline bool WithinCloseOpenRange(const T &x, const T &minv, const T &maxv)
{
  return (x >= minv) && (x < maxv);
}

/*
template<typename T>
inline T ClipValue(const T &v, const T &lower, const T&upper)
{
  return (v < lower) ? lower : (v > upper) ? upper : v;
}

template<typename T>
inline bool ClipRange(const T &lower, const T&upper, T &x0, T &x1)
{
  if (x0 <= x1) {
    if (x0 <= upper && x1 >= lower) {
      if (x0 < lower) {
        x0 = lower;
      }
      if (x1 > upper) {
        x1 = upper;
      }
      return true;
    }
  }

  return false;
}
*/

/*!
 * \brief Process partitions of a range
 *
 * The behavior of this function is undefined if x0 > x1 or n <= 0.
 *
 * \param x0 Min value of the range.
 * \param x1 Max value of the range.
 * \param n Number of partitions.
 * \param f Function to process the partition. It takes min and max of a partition
 *          as its parameters.
 */
void RangePartitionProcess(
    int x0, int x1, int n, std::function<void(int, int)> f);

//void RangePartitionProcess(
//    int x0, int x1, int block, int n, std::function<void(int, int)> f);

class ApplyOnce {
public:
  ApplyOnce(std::function<void()> startFunc, std::function<void()> endFunc) {
    startFunc();
    m_endFunc = endFunc;
  }

  ~ApplyOnce() {
    m_endFunc();
  }

private:
  std::function<void()> m_endFunc;
};

} //namespace neutu

//template<>
//int numDigits(int32_t x);

template<typename T>
std::set<T> neutu::intersect(const std::set<T> &s1, const std::set<T> &s2)
{
  std::set<T> result;
  std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.begin()));
  return result;
}

template<typename T>
std::set<T> neutu::setdiff(const std::set<T> &s1, const std::set<T> &s2)
{
  std::set<T> result;
  std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                      std::inserter(result, result.begin()));
  return result;
}

template<typename T>
void neutu::assign(T *out, const T &v)
{
  if (out != NULL) {
    *out = v;
  }
}

#endif // UTILITIES_H
