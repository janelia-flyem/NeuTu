#ifndef NEUTU_COMMON_MATH_H
#define NEUTU_COMMON_MATH_H

namespace neutu {

template<typename T>
int iround(const T &v);

extern template
int iround<double>(const double &v);

extern template
int iround<float>(const float &v);

/*! Round to the closest bigger integer
 *
 * -0.5 -> 0 instead of -1
 */
template<typename T>
int nround(const T &v);

extern template
int nround<double>(const double &v);

extern template
int nround<float>(const float &v);

/*! Round to the closest bigger integer
 *
 *  a variant of \a nround, but 0.5 -> 0 instead of 1. Similarly, -0.5 -> -1.
 */
template<typename T>
int hround(const T &v);

extern template
int hround<double>(const double &v);

extern template
int hround<float>(const float &v);

template<typename T>
int ifloor(const T &v);

extern template
int ifloor<double>(const double &v);

extern template
int ifloor<float>(const float &v);

template<typename T>
int iceil(const T &v);

extern template
int iceil<double>(const double &v);

extern template
int iceil<float>(const float &v);


}



#endif // NEUTU_COMMON_MATH_H
