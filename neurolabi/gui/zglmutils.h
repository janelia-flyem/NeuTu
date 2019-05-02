#ifndef ZGLMUTILS_H
#define ZGLMUTILS_H

// This file includes some commonly used headers from glm and defines some useful functions
// for glm

//#if defined(_CPP11_)
#if __cplusplus >= 201103L
#define GLM_FORCE_CXX11
#endif

#define GLM_FORCE_SSE3
//#define GLM_FORCE_INLINE
#define GLM_FORCE_SIZE_T_LENGTH
#define GLM_FORCE_NO_CTOR_INIT
#define GLM_FORCE_EXPLICIT_CTOR
//#define GLM_FORCE_MESSAGES
#define GLM_FORCE_SWIZZLE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <QRegularExpression>
#include <QStringList>
#include <QColor>
#include <QLocale>
#include <QDebug>
#include <iostream>
#include <sstream>
#include <tuple>

namespace glm {

using col3 = vec<3, unsigned char, highp>;
using col4 = vec<4, unsigned char, highp>;

// apply transform matrix
template<typename T, precision P>
vec<3, T, P> applyMatrix(const mat<4, 4, T, P>& m, const vec<3, T, P>& v)
{
  vec<4, T, P> res = m * vec<4, T, P>(v, T(1));
  return vec<3, T, P>(res / res.w);
}

// given vec, get normalized vector e1 and e2 to make (e1,e2,vec) orthogonal to each other
// **crash** if vec is zero
template<typename T, precision P>
void getOrthogonalVectors(const vec<3, T, P>& v, vec<3, T, P>& e1, vec<3, T, P>& e2)
{
  static_assert(std::numeric_limits<T>::is_iec559, "'getOrthogonalVectors' only accept floating-point inputs");
  T eps = std::numeric_limits<T>::epsilon() * 1e2;

  e1 = cross(v, vec<3, T, P>(T(1), T(0), T(0)));
  if (dot(e1, e1) < eps)
    e1 = cross(v, vec<3, T, P>(T(0), T(1), T(0)));
  e1 = normalize(e1);
  e2 = normalize(cross(e1, v));
}

inline quat mix(const quat& q1, const quat& q2, double p)
{
  return mix(q1, q2, float(p));
}

} // namespace glm

template<typename T, glm::precision P>
class Vec2Compare
{
  bool less;
public:
  explicit Vec2Compare(bool less_ = true)
    : less(less_)
  {}

  bool operator()(const glm::vec<2, T, P>& lhs, const glm::vec<2, T, P>& rhs) const
  {
    if (less) {
      return std::tie(lhs.y, lhs.x) < std::tie(rhs.y, rhs.x);
    } else {
      return std::tie(lhs.y, lhs.x) > std::tie(rhs.y, rhs.x);
    }
  }
};

template<typename T, glm::precision P>
class Vec3Compare
{
  bool less;
public:
  explicit Vec3Compare(bool less_ = true)
    : less(less_)
  {}

  bool operator()(const glm::vec<3, T, P>& lhs, const glm::vec<3, T, P>& rhs) const
  {
    if (less) {
      return std::tie(lhs.z, lhs.y, lhs.x) < std::tie(rhs.z, rhs.y, rhs.x);
    } else {
      return std::tie(lhs.z, lhs.y, lhs.x) > std::tie(rhs.z, rhs.y, rhs.x);
    }
  }
};

template<typename T, glm::precision P>
class Vec4Compare
{
  bool less;
public:
  explicit Vec4Compare(bool less_ = true)
    : less(less_)
  {}

  bool operator()(const glm::vec<4, T, P>& lhs, const glm::vec<4, T, P>& rhs) const
  {
    if (less) {
      return std::tie(lhs.w, lhs.z, lhs.y, lhs.x) < std::tie(rhs.w, rhs.z, rhs.y, rhs.x);
    } else {
      return std::tie(lhs.w, lhs.z, lhs.y, lhs.x) > std::tie(rhs.w, rhs.z, rhs.y, rhs.x);
    }
  }
};

using Col3Compare = Vec3Compare<unsigned char, glm::highp>;
using Col4Compare = Vec4Compare<unsigned char, glm::highp>;

// serialization support

inline void toVal(const QString& str, bool& v)
{
  v = QString::compare(str, "false", Qt::CaseInsensitive) != 0;
}

inline void toVal(const QString& str, QString& v)
{
  v = str;
}

inline void toVal(const QString& str, float& v)
{
  v = str.toFloat();
}

inline void toVal(const QString& str, double& v)
{
  v = str.toDouble();
}

inline void toVal(const QString& str, int& v)
{
  v = str.toInt();
}

inline void toVal(const QString& str, unsigned char& v)
{
  v = str.toUShort();
}

inline void toVal(const QString& str, size_t& v)
{
  v = str.toULongLong();
}

template<typename T>
inline void toVal(const std::string& str, T& v)
{
  toVal(QString::fromStdString(str), v);
}

template<typename T>
inline QString toQString(T v)
{
  static_assert(std::is_integral<T>::value, "Integer required.");
  return QString::number(v);
}

inline QString toQString(float v)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  return QString::number(v, 'g', QLocale::FloatingPointShortest);
#else
  return QString::number(v, 'g', std::numeric_limits<float>::max_digits10);
#endif
}

inline QString toQString(double v)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  return QString::number(v, 'g', QLocale::FloatingPointShortest);
#else
  return QString::number(v, 'g', std::numeric_limits<double>::max_digits10);
#endif
}

inline QString toQString(const QString& v)
{
  return v;
}

template<size_t L, typename T, glm::precision P>
inline QString toQString(const glm::vec<L, T, P>& v)
{
  static_assert(std::is_integral<T>::value, "Integer required.");
  QString res = "[" + QString::number(v[0]);
  for (size_t i = 1; i < L; ++i) {
    res += ", ";
    res += QString::number(v[i]);
  }
  res += "]";
  return res;
}

template<size_t L, glm::precision P>
inline QString toQString(const glm::vec<L, float, P>& v)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  QString res = "[" + QString::number(v[0], 'g', QLocale::FloatingPointShortest);
  for (size_t i = 1; i < L; ++i) {
    res += ", ";
    res += QString::number(v[i], 'g', QLocale::FloatingPointShortest);
  }
  res += "]";
  return res;
#else
  QString res = "[" + QString::number(v[0], 'g', std::numeric_limits<float>::max_digits10);
  for (size_t i = 1; i < L; ++i) {
    res += ", ";
    res += QString::number(v[i], 'g', std::numeric_limits<float>::max_digits10);
  }
  res += "]";
  return res;
#endif
}

template<size_t L, glm::precision P>
inline QString toQString(const glm::vec<L, double, P>& v)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  QString res = "[" + QString::number(v[0], 'g', QLocale::FloatingPointShortest);
  for (size_t i = 1; i < L; ++i) {
    res += ", ";
    res += QString::number(v[i], 'g', QLocale::FloatingPointShortest);
  }
  res += "]";
  return res;
#else
  QString res = "[" + QString::number(v[0], 'g', std::numeric_limits<double>::max_digits10);
  for (size_t i = 1; i < L; ++i) {
    res += ", ";
    res += QString::number(v[i], 'g', std::numeric_limits<double>::max_digits10);
  }
  res += "]";
  return res;
#endif
}

template<size_t L, typename T, glm::precision P>
inline void toVal(const QString& str, glm::vec<L, T, P>& v)
{
  QRegularExpression rx(R"((\ |\,|\[|\]|\;))"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (size_t i = 0; i < std::min(L, size_t(numList.size())); ++i) {
    toVal(numList[i], v[i]);
  }
}

inline QString toQString(const QColor& v)
{
  return "[" + QString::number(v.red()) +
         ", " + QString::number(v.green()) +
         ", " + QString::number(v.blue()) +
         ", " + QString::number(v.alpha()) +
         "]";
}

inline void toVal(const QString& str, QColor& v)
{
  QRegularExpression rx(R"((\ |\,|\[|\]|\;))"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i = 0; i < std::min(4, numList.size()); ++i) {
    int c;
    toVal(numList[i], c);
    if (i == 0) {
      v.setRed(c);
    } else if (i == 1) {
      v.setGreen(c);
    } else if (i == 2) {
      v.setBlue(c);
    } else if (i == 3) {
      v.setAlpha(c);
    }
  }
}

template<size_t C, size_t R, typename T, glm::precision P>
inline QString toQString(const glm::mat<C, R, T, P>& m)
{
  static_assert(std::is_integral<T>::value, "Integer required.");
  QString res = "[";
  for (size_t r = 0; r < R; ++r) {
    if (r > 0)
      res += "; ";
    for (size_t c = 0; c < C; ++c) {
      if (c > 0)
        res += ", ";
      res += QString::number(m[c][r]);
    }
  }
  res += "]";
  return res;
}

template<size_t C, size_t R, glm::precision P>
inline QString toQString(const glm::mat<C, R, float, P>& m)
{
  QString res = "[";
  for (size_t r = 0; r < R; ++r) {
    if (r > 0)
      res += "; ";
    for (size_t c = 0; c < C; ++c) {
      if (c > 0)
        res += ", ";
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
      res += QString::number(m[c][r], 'g', QLocale::FloatingPointShortest);
#else
      res += QString::number(m[c][r], 'g', std::numeric_limits<float>::max_digits10);
#endif
    }
  }
  res += "]";
  return res;
}

template<size_t C, size_t R, glm::precision P>
inline QString toQString(const glm::mat<C, R, double, P>& m)
{
  QString res = "[";
  for (size_t r = 0; r < R; ++r) {
    if (r > 0)
      res += "; ";
    for (size_t c = 0; c < C; ++c) {
      if (c > 0)
        res += ", ";
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
      res += QString::number(m[c][r], 'g', QLocale::FloatingPointShortest);
#else
      res += QString::number(m[c][r], 'g', std::numeric_limits<double>::max_digits10);
#endif
    }
  }
  res += "]";
  return res;
}

template<size_t C, size_t R, typename T, glm::precision P>
inline void toVal(const QString& str, glm::mat<C, R, T, P>& m)
{
  QRegularExpression rx(R"((\ |\,|\[|\]|\;))"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (size_t i = 0; i < std::min(C * R, size_t(numList.size())); ++i) {
    toVal(numList[i], m[i % C][i / R]);
  }
}

template<typename T, glm::precision P>
inline QString toQString(const glm::tquat<T, P>& v)
{
  static_assert(std::is_integral<T>::value, "Integer required.");
  return "[" + QString::number(v[0]) +
         ", " + QString::number(v[1]) +
         ", " + QString::number(v[2]) +
         ", " + QString::number(v[3]) +
         "]";
}

template<glm::precision P>
inline QString toQString(const glm::tquat<float, P>& v)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  return "[" + QString::number(v[0], 'g', QLocale::FloatingPointShortest) +
         ", " + QString::number(v[1], 'g', QLocale::FloatingPointShortest) +
         ", " + QString::number(v[2], 'g', QLocale::FloatingPointShortest) +
         ", " + QString::number(v[3], 'g', QLocale::FloatingPointShortest) +
         "]";
#else
  return "[" + QString::number(v[0], 'g', std::numeric_limits<float>::max_digits10) +
         ", " + QString::number(v[1], 'g', std::numeric_limits<float>::max_digits10) +
         ", " + QString::number(v[2], 'g', std::numeric_limits<float>::max_digits10) +
         ", " + QString::number(v[3], 'g', std::numeric_limits<float>::max_digits10) +
         "]";
#endif
}

template<glm::precision P>
inline QString toQString(const glm::tquat<double, P>& v)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 7, 0)
  return "[" + QString::number(v[0], 'g', QLocale::FloatingPointShortest) +
         ", " + QString::number(v[1], 'g', QLocale::FloatingPointShortest) +
         ", " + QString::number(v[2], 'g', QLocale::FloatingPointShortest) +
         ", " + QString::number(v[3], 'g', QLocale::FloatingPointShortest) +
         "]";
#else
  return "[" + QString::number(v[0], 'g', std::numeric_limits<double>::max_digits10) +
         ", " + QString::number(v[1], 'g', std::numeric_limits<double>::max_digits10) +
         ", " + QString::number(v[2], 'g', std::numeric_limits<double>::max_digits10) +
         ", " + QString::number(v[3], 'g', std::numeric_limits<double>::max_digits10) +
         "]";
#endif
}

template<typename T, glm::precision P>
inline void toVal(const QString& str, glm::tquat<T, P>& q)
{
  QRegularExpression rx(R"((\ |\,|\[|\]|\;))"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (size_t i = 0; i < std::min(q.length(), size_t(numList.size())); ++i) {
    toVal(numList[i], q[i]);
  }
}

//-------------------------------------------------------------------------------------------------------------------------
// qDebug print

template<size_t L, typename T, glm::precision P>
inline QDebug& operator<<(QDebug s, const glm::vec<L, T, P>& v)
{
  s.nospace() << qUtf8Printable(toQString(v));
  return s.space();
}

template<size_t C, size_t R, typename T, glm::precision P>
inline QDebug& operator<<(QDebug s, const glm::mat<C, R, T, P>& m)
{
  s.nospace() << qUtf8Printable(toQString(m));
  return s.space();
}

template<typename T, glm::precision P>
inline QDebug& operator<<(QDebug s, const glm::tquat<T, P>& q)
{
  s.nospace() << qUtf8Printable(toQString(q));
  return s.space();
}

//-------------------------------------------------------------------------------------------------------------------------

//-------------------------------------------------------------------------------------------------------------------------
// std iostream print

template<size_t L, typename T, glm::precision P>
inline std::ostream& operator<<(std::ostream& s, const glm::vec<L, T, P>& v)
{
  return (s << qUtf8Printable(toQString(v)));
}

template<size_t C, size_t R, typename T, glm::precision P>
inline std::ostream& operator<<(std::ostream& s, const glm::mat<C, R, T, P>& m)
{
  return (s << qUtf8Printable(toQString(m)));
}

template<typename T, glm::precision P>
inline std::ostream& operator<<(std::ostream& s, const glm::tquat<T, P>& q)
{
  return (s << qUtf8Printable(toQString(q)));
}

//-------------------------------------------------------------------------------------------------------------------------

#endif // ZGLMUTILS_H
