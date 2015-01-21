#ifndef ZGLMUTILS_H
#define ZGLMUTILS_H

// This file includes some commonly used headers from glm and defines some useful functions
// for glm

#define GLM_FORCE_SSE2
//#define GLM_FORCE_INLINE
#define GLM_FORCE_SIZE_T_LENGTH
#define GLM_FORCE_SIZE_FUNC
#define GLM_FORCE_NO_CTOR_INIT
#define GLM_FORCE_EXPLICIT_CTOR
//#define GLM_MESSAGES
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <iostream>
#include <sstream>
#include <QDebug>
#include <QRegExp>
#include <QStringList>
#include <QColor>

namespace glm {
typedef tvec3<unsigned char, highp> col3;
typedef tvec4<unsigned char, highp> col4;

// apply transform matrix
template<typename T, precision P>
tvec3<T,P> applyMatrix(const tmat4x4<T,P> &mat, const tvec3<T,P> &vec)
{
  tvec4<T,P> res = mat * tvec4<T,P>(vec, T(1));
  return tvec3<T,P>(res / res.w);
}

// given vec, get normalized vector e1 and e2 to make (e1,e2,vec) orthogonal to each other
// **crash** if vec is zero
template<typename T, precision P>
void getOrthogonalVectors(const tvec3<T,P> &vec, tvec3<T,P> &e1, tvec3<T,P> &e2)
{
  GLM_STATIC_ASSERT(std::numeric_limits<T>::is_iec559, "'getOrthogonalVectors' only accept floating-point inputs");
  T eps = std::numeric_limits<T>::epsilon() * 1e2;

  e1 = cross(vec, tvec3<T,P>(T(1), T(0), T(0)));
  if (dot(e1, e1) < eps)
    e1 = cross(vec, tvec3<T,P>(T(0), T(1), T(0)));
  e1 = normalize(e1);
  e2 = normalize(cross(e1, vec));
}

inline quat mix(const quat &q1, const quat &q2, double p)
{
  return mix(q1, q2, float(p));
}

}

template<typename T, glm::precision P>
class Vec2Compare
{
  bool less;
public:
  Vec2Compare(bool less = true) : less(less) {}
  bool operator() (const glm::tvec2<T,P>& lhs, const glm::tvec2<T,P>& rhs) const
  {
    if (less) {
      if (lhs.y != rhs.y)
        return lhs.y < rhs.y;
      return lhs.x < rhs.x;
    } else {
      if (lhs.y != rhs.y)
        return lhs.y > rhs.y;
      return lhs.x > rhs.x;
    }
  }
};

template<typename T, glm::precision P>
class Vec3Compare
{
  bool less;
public:
  Vec3Compare(bool less = true) : less(less) {}
  bool operator() (const glm::tvec3<T,P>& lhs, const glm::tvec3<T,P>& rhs) const
  {
    if (less) {
      if (lhs.z != rhs.z)
        return lhs.z < rhs.z;
      if (lhs.y != rhs.y)
        return lhs.y < rhs.y;
      return lhs.x < rhs.x;
    } else {
      if (lhs.z != rhs.z)
        return lhs.z > rhs.z;
      if (lhs.y != rhs.y)
        return lhs.y > rhs.y;
      return lhs.x > rhs.x;
    }
  }
};

template<typename T, glm::precision P>
class Vec4Compare
{
  bool less;
public:
  Vec4Compare(bool less = true) : less(less) {}
  bool operator() (const glm::tvec4<T,P>& lhs, const glm::tvec4<T,P>& rhs) const
  {
    if (less) {
      if (lhs.w != rhs.w)
        return lhs.w < rhs.w;
      if (lhs.z != rhs.z)
        return lhs.z < rhs.z;
      if (lhs.y != rhs.y)
        return lhs.y < rhs.y;
      return lhs.x < rhs.x;
    } else {
      if (lhs.w != rhs.w)
        return lhs.w > rhs.w;
      if (lhs.z != rhs.z)
        return lhs.z > rhs.z;
      if (lhs.y != rhs.y)
        return lhs.y > rhs.y;
      return lhs.x > rhs.x;
    }
  }
};

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
  return QString("%1").arg(v);
}

template<typename T, glm::precision P>
inline QString toQString(const glm::tvec2<T,P>& v)
{
  return QString("[%1, %2]").arg(v[0]).arg(v[1]);
}

template<typename T, glm::precision P>
inline void toVal(const QString &str, glm::tvec2<T,P>& v)
{
  QRegExp rx("(\\ |\\,|\\[|\\]|\\;)"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i=0; i<std::min(2,numList.size()); ++i) {
    toVal(numList[i], v[i]);
  }
}

template<typename T, glm::precision P>
inline QString toQString(const glm::tvec3<T,P>& v)
{
  return QString("[%1, %2, %3]").arg(v[0]).arg(v[1]).arg(v[2]);
}

template<typename T, glm::precision P>
inline void toVal(const QString &str, glm::tvec3<T,P>& v)
{
  QRegExp rx("(\\ |\\,|\\[|\\]|\\;)"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i=0; i<std::min(3,numList.size()); ++i) {
    toVal(numList[i], v[i]);
  }
}

template<typename T, glm::precision P>
inline QString toQString(const glm::tvec4<T,P>& v)
{
  return QString("[%1, %2, %3, %4]").arg(v[0]).arg(v[1]).arg(v[2]).arg(v[3]);
}

template<typename T, glm::precision P>
inline void toVal(const QString &str, glm::tvec4<T,P>& v)
{
  QRegExp rx("(\\ |\\,|\\[|\\]|\\;)"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i=0; i<std::min(4,numList.size()); ++i) {
    toVal(numList[i], v[i]);
  }
}

inline QString toQString(const QColor& v)
{
  return QString("[%1, %2, %3, %4]").arg(v.red()).arg(v.green()).arg(v.blue()).arg(v.alpha());
}

inline void toVal(const QString &str, QColor& v)
{
  QRegExp rx("(\\ |\\,|\\[|\\]|\\;)"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i=0; i<std::min(4,numList.size()); ++i) {
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


template<typename T, glm::precision P>
inline QString toQString(const glm::tmat2x2<T,P> &m)
{
  return QString("[%1, %2; %3, %4]").
      arg(m[0][0]).arg(m[1][0]).
      arg(m[0][1]).arg(m[1][1]);
}

template<typename T, glm::precision P>
inline void toVal(const QString &str, glm::tmat2x2<T,P>& m)
{
  QRegExp rx("(\\ |\\,|\\[|\\]|\\;)"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i=0; i<std::min(4,numList.size()); ++i) {
    toVal(numList[i], m[i%2][i/2]);
  }
}

template<typename T, glm::precision P>
inline QString toQString(const glm::tmat3x3<T,P> &m)
{
  return QString("[%1, %2, %3; %4, %5, %6; %7, %8, %9]").
      arg(m[0][0]).arg(m[1][0]).arg(m[2][0]).
      arg(m[0][1]).arg(m[1][1]).arg(m[2][1]).
      arg(m[0][2]).arg(m[1][2]).arg(m[2][2]);
}

template<typename T, glm::precision P>
inline void toVal(const QString &str, glm::tmat3x3<T,P>& m)
{
  QRegExp rx("(\\ |\\,|\\[|\\]|\\;)"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i=0; i<std::min(9,numList.size()); ++i) {
    toVal(numList[i], m[i%3][i/3]);
  }
}

template<typename T, glm::precision P>
inline QString toQString(const glm::tmat4x4<T,P> &m)
{
  return QString("[%1, %2, %3, %4; %5, %6, %7, %8; %9, %10, %11, %12; %13, %14, %15, %16]").
      arg(m[0][0]).arg(m[1][0]).arg(m[2][0]).arg(m[3][0]).
      arg(m[0][1]).arg(m[1][1]).arg(m[2][1]).arg(m[3][1]).
      arg(m[0][2]).arg(m[1][2]).arg(m[2][2]).arg(m[3][2]).
      arg(m[0][3]).arg(m[1][3]).arg(m[2][3]).arg(m[3][3]);
}

template<typename T, glm::precision P>
inline void toVal(const QString &str, glm::tmat4x4<T,P>& m)
{
  QRegExp rx("(\\ |\\,|\\[|\\]|\\;)"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i=0; i<std::min(16,numList.size()); ++i) {
    toVal(numList[i], m[i%4][i/4]);
  }
}

template<typename T, glm::precision P>
inline QString toQString(const glm::tquat<T,P> &q)
{
  return QString("[%1, %2, %3, %4]").arg(q[0]).arg(q[1]).arg(q[2]).arg(q[3]);
}

template<typename T, glm::precision P>
inline void toVal(const QString &str, glm::tquat<T,P>& q)
{
  QRegExp rx("(\\ |\\,|\\[|\\]|\\;)"); //RegEx for ' ' or ',' or '[' or ']' or ';'
  QStringList numList = str.split(rx, QString::SkipEmptyParts);
  for (int i=0; i<std::min(4,numList.size()); ++i) {
    toVal(numList[i], q[i]);
  }
}

//-------------------------------------------------------------------------------------------------------------------------
// std iostream print

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::tvec2<T,P>& v)
{
  return (s << qPrintable(toQString(v)));
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::tvec3<T,P>& v)
{
  return (s << qPrintable(toQString(v)));
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::tvec4<T,P>& v)
{
  return (s << qPrintable(toQString(v)));
}

template<>
inline std::ostream& operator << <unsigned char,glm::highp>(std::ostream& s, const glm::tvec2<unsigned char,glm::highp>& v)
{
  return (s << glm::tvec2<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <unsigned char,glm::highp>(std::ostream& s, const glm::tvec3<unsigned char,glm::highp>& v)
{
  return (s << glm::tvec3<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <unsigned char,glm::highp>(std::ostream& s, const glm::tvec4<unsigned char,glm::highp>& v)
{
  return (s << glm::tvec4<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <char,glm::highp>(std::ostream& s, const glm::tvec2<char,glm::highp>& v)
{
  return (s << glm::tvec2<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <char,glm::highp>(std::ostream& s, const glm::tvec3<char,glm::highp>& v)
{
  return (s << glm::tvec3<int,glm::highp>(v));
}

template<>
inline std::ostream& operator << <char,glm::highp>(std::ostream& s, const glm::tvec4<char,glm::highp>& v)
{
  return (s << glm::tvec4<int,glm::highp>(v));
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::tmat2x2<T,P>& m)
{
  return (s << qPrintable(toQString(m)));
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::tmat3x3<T,P>& m)
{
  return (s << qPrintable(toQString(m)));
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::tmat4x4<T,P>& m)
{
  return (s << qPrintable(toQString(m)));
}

template<typename T, glm::precision P>
std::ostream& operator << (std::ostream& s, const glm::tquat<T,P>& q)
{
  return (s << qPrintable(toQString(q)));
}

//-------------------------------------------------------------------------------------------------------------------------
// qDebug print

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::tvec2<T,P>& v)
{
  s.nospace() << qPrintable(toQString(v));
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::tvec3<T,P>& v)
{
  s.nospace() << qPrintable(toQString(v));
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::tvec4<T,P>& v)
{
  s.nospace() << qPrintable(toQString(v));
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::tmat2x2<T,P>& m)
{
  s.nospace() << qPrintable(toQString(m));
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug& s, const glm::tmat3x3<T,P>& m)
{
  s.nospace() << qPrintable(toQString(m));
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::tmat4x4<T,P>& m)
{
  s.nospace() << qPrintable(toQString(m));
  return s.space();
}

template<typename T, glm::precision P>
QDebug& operator << (QDebug s, const glm::tquat<T,P>& q)
{
  s.nospace() << qPrintable(toQString(q));
  return s.space();
}

//-------------------------------------------------------------------------------------------------------------------------

#endif // ZGLMUTILS_H
