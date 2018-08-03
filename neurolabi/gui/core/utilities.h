#ifndef CORE_UTILITIES_H
#define CORE_UTILITIES_H

#include <set>
#include <algorithm>
#include <iostream>

#define NT_STR(s) #s
#define NT_XSTR(s) NT_STR(s)

namespace neutube
{
template<typename T>
void read(std::istream &stream, T &v)
{
  stream.read((char*)(&v), sizeof(T));
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

template<typename T>
void assign(T *out, const T &v);

template<typename T>
std::set<T> intersect(const std::set<T> &s1, const std::set<T> &s2);

template<typename T>
std::set<T> setdiff(const std::set<T> &s1, const std::set<T> &s2);

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

}

//template<>
//int numDigits(int32_t x);

template<typename T>
std::set<T> neutube::intersect(const std::set<T> &s1, const std::set<T> &s2)
{
  std::set<T> result;
  std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.begin()));
  return result;
}

template<typename T>
std::set<T> neutube::setdiff(const std::set<T> &s1, const std::set<T> &s2)
{
  std::set<T> result;
  std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                      std::inserter(result, result.begin()));
  return result;
}

template<typename T>
void neutube::assign(T *out, const T &v)
{
  if (out != NULL) {
    *out = v;
  }
}

#endif // UTILITIES_H
