#include "math.h"

#include <cmath>

template<typename T>
int neutu::iround(const T &v)
{
  return std::lround(v);
}

template
int neutu::iround<double>(const double &v);

template
int neutu::iround<float>(const float &v);

template<typename T>
int neutu::nround(const T &v)
{
  return int(std::floor(v + 0.5));
}

template
int neutu::nround<double>(const double &v);

template
int neutu::nround<float>(const float &v);

template<typename T>
int neutu::hround(const T &v)
{
  if (v - std::floor(v) == 0.5) {
    return int(std::floor(v));
  }

  return int(std::floor(v + 0.5));
}

template
int neutu::hround<double>(const double &v);

template
int neutu::hround<float>(const float &v);

template<typename T>
int neutu::ifloor(const T &v)
{
  return int(std::floor(v));
}

template
int neutu::ifloor<double>(const double &v);

template
int neutu::ifloor<float>(const float &v);

template<typename T>
int neutu::iceil(const T &v)
{
  return int(std::ceil(v));
}

template
int neutu::iceil<double>(const double &v);

template
int neutu::iceil<float>(const float &v);
