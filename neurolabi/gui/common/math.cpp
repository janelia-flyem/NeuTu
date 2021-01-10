#include "math.h"

#include <cmath>

template<typename T>
int neutu::iround(const T &v)
{
  return int(std::round(v));
}

template
int neutu::iround<double>(const double &v);

template
int neutu::iround<float>(const float &v);

template<typename T>
int neutu::ifloor(const T &v)
{
  return int(std::floor(v));
}

template
int neutu::ifloor<double>(const double &v);

template
int neutu::ifloor<float>(const float &v);
