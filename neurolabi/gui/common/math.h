#ifndef NEUTU_COMMON_MATH_H
#define NEUTU_COMMON_MATH_H

namespace neutu {

template<typename T>
int iround(const T &v);

extern template
int iround<double>(const double &v);

extern template
int iround<float>(const float &v);

template<typename T>
int ifloor(const T &v);

extern template
int ifloor<double>(const double &v);

extern template
int ifloor<float>(const float &v);

}



#endif // NEUTU_COMMON_MATH_H
