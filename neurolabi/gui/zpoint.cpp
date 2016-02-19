#include "zpoint.h"

#include <iostream>
#include <sstream>

#if defined(_QT_GUI_USED_)
#include <QPainter>
#endif

#include <stdio.h>

#include "tz_math.h"
#include "tz_geo3d_utils.h"
#include "tz_coordinate_3d.h"
#include "tz_error.h"
#include <cstdio>
#include "tz_geo3d_utils.h"
#include "zintpoint.h"
#include "geometry/zgeometry.h"

const double ZPoint::m_minimalDistance = 1e-5;

ZPoint::ZPoint()
{
  set(0, 0, 0);
}

ZPoint::ZPoint(double x, double y, double z)
{
  set(x, y, z);
}

ZPoint::ZPoint(const double *pt)
{
  set(pt[0], pt[1], pt[2]);
}

ZPoint::ZPoint(const ZPoint &pt)
{
  set(pt.m_x, pt.m_y, pt.m_z);
}

#if 0
void ZPoint::display(ZPainter &painter, int n, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  if ((iround(m_z) == n) || (n == -1)) {
    painter.setPen(QPen(QColor(0, 0, 255, 255), .7));
    painter.drawPoint(m_x, m_y);
  }
#else
  UNUSED_PARAMETER(&painter);
  UNUSED_PARAMETER(n);
  UNUSED_PARAMETER(style);
#endif
}
#endif

void ZPoint::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZPoint::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

double ZPoint::distanceTo(const ZPoint &pt) const
{
  return Geo3d_Dist(m_x, m_y, m_z, pt.x(), pt.y(), pt.z());
}

double ZPoint::distanceTo(double x, double y, double z) const
{
  return Geo3d_Dist(m_x, m_y, m_z, x, y, z);
}

double ZPoint::length() const
{
  return Geo3d_Orgdist(m_x, m_y, m_z);
}

ZPoint& ZPoint::operator +=(const ZPoint &pt)
{
  m_x += pt.m_x;
  m_y += pt.m_y;
  m_z += pt.m_z;

  return *this;
}

ZPoint& ZPoint::operator +=(const ZIntPoint &pt)
{
  m_x += pt.getX();
  m_y += pt.getY();
  m_z += pt.getZ();

  return *this;
}

ZPoint& ZPoint::operator -=(const ZPoint &pt)
{
  m_x -= pt.m_x;
  m_y -= pt.m_y;
  m_z -= pt.m_z;

  return *this;
}

ZPoint& ZPoint::operator *=(const ZPoint &pt)
{
  m_x *= pt.m_x;
  m_y *= pt.m_y;
  m_z *= pt.m_z;

  return *this;
}

ZPoint& ZPoint::operator +=(double offset)
{
  m_x += offset;
  m_y += offset;
  m_z += offset;

  return *this;
}

ZPoint& ZPoint::operator -=(double offset)
{
  m_x -= offset;
  m_y -= offset;
  m_z -= offset;

  return *this;
}

ZPoint& ZPoint::operator *=(double scale)
{
  m_x *= scale;
  m_y *= scale;
  m_z *= scale;

  return *this;
}

ZPoint& ZPoint::operator /=(double scale)
{
  m_x /= scale;
  m_y /= scale;
  m_z /= scale;

  return *this;
}

ZPoint& ZPoint::operator /=(const ZPoint &pt)
{
  m_x /= pt.m_x;
  m_y /= pt.m_y;
  m_z /= pt.m_z;

  return *this;
}

ZPoint operator + (const ZPoint &pt1, const ZPoint &pt2)
{
  return ZPoint(pt1) += pt2;
}

ZPoint operator + (const ZPoint &pt1, const ZIntPoint &pt2)
{
  return ZPoint(pt1) += pt2;
}

ZPoint operator - (const ZPoint &pt1, const ZPoint &pt2)
{
  return ZPoint(pt1) -= pt2;
}

ZPoint operator * (const ZPoint &pt1, double scale)
{
  return ZPoint(pt1) *= scale;
}

ZPoint operator / (const ZPoint &pt1, double scale)
{
  return ZPoint(pt1) /= scale;
}


void ZPoint::toArray(double *pt) const
{
  pt[0] = m_x;
  pt[1] = m_y;
  pt[2] = m_z;
}

void ZPoint::normalize()
{
  coordinate_3d_t coord;
  toArray(coord);
  Coordinate_3d_Unitize(coord);
  set(coord[0], coord[1], coord[2]);
}

const double& ZPoint::operator [](int index) const
{
  TZ_ASSERT(index >= 0 && index < 3, "Invalid index");

  switch (index) {
  case 0:
    return m_x;
  case 1:
    return m_y;
  case 2:
    return m_z;
  default:
    break;
  }

  std::cerr << "Index out of bound" << std::endl;

  return m_x;
}

double& ZPoint::operator[] (int index)
{
  return const_cast<double&>(static_cast<const ZPoint&>(*this)[index]);
}

ZPoint& ZPoint::operator= (const ZPoint &pt)
{
  m_x = pt.m_x;
  m_y = pt.m_y;
  m_z = pt.m_z;

  return *this;
}

bool ZPoint::operator ==(const ZPoint &pt)
{
  return this->x() == pt.x() && this->y() == pt.y() && this->z() == pt.z();
}

double ZPoint::dot(const ZPoint &pt) const
{
  return m_x * pt.x() + m_y * pt.y() + m_z * pt.z();
}

ZPoint ZPoint::cross(const ZPoint &pt) const
{
  return ZPoint(y() * pt.z() - pt.y() * z(),
                pt.x() * z() - x() * pt.z(),
                x() * pt.y() - pt.x() * y());
}

double ZPoint::cosAngle(const ZPoint &pt) const
{
  if (this->isApproxOrigin() || pt.isApproxOrigin()) {
    return 1.0;
  }

  double value = dot(pt) / length() / pt.length();

  if (value > 1.0) {
    value = 1.0;
  } else if (value < -1.0) {
    value = -1.0;
  }

  return value;
}

bool ZPoint::isApproxOrigin() const
{
  return (length() < m_minimalDistance);
}

bool ZPoint::approxEquals(const ZPoint &pt) const
{
  return (distanceTo(pt) < m_minimalDistance);
}

std::string ZPoint::toString() const
{
  std::ostringstream stream;
  stream << "(" << x() << ", " << y() << ", " << z() << ")";

  return stream.str();
}

std::string ZPoint::toJsonString() const
{
  std::ostringstream stream;
  stream << "[" << x() << ", " << y() << ", " << z() << "]";

  return stream.str();
}

void ZPoint::print() const
{
  std::cout << toString() << std::endl;
}

ZPoint ZPoint::operator - () const
{
  return ZPoint(-x(), -y(), -z());
}

ZIntPoint ZPoint::toIntPoint() const
{
  return ZIntPoint(iround(x()), iround(y()), iround(z()));
}

ZIntPoint& ZIntPoint::operator +=(const ZIntPoint &pt)
{
  m_x += pt.getX();
  m_y += pt.getY();
  m_z += pt.getZ();

  return *this;
}

ZIntPoint& ZIntPoint::operator -=(const ZIntPoint &pt)
{
  m_x -= pt.getX();
  m_y -= pt.getY();
  m_z -= pt.getZ();

  return *this;
}

ZIntPoint& ZIntPoint::operator *=(const ZIntPoint &pt)
{
  m_x *= pt.getX();
  m_y *= pt.getY();
  m_z *= pt.getZ();

  return *this;
}

void ZPoint::rotate(double theta, double psi)
{
  Geo3d_Rotate_Coordinate(&(m_x), &(m_y), &(m_z),
                          theta, psi, FALSE);
}

void ZPoint::translate(const ZPoint &dp)
{
  translate(dp.x(), dp.y(), dp.z());
}

void ZPoint::rotate(double theta, double psi, const ZPoint &center)
{
  translate(-center);
  rotate(theta, psi);
  translate(center);
}

bool ZPoint::operator <(const ZPoint &pt) const
{
  if (z() < pt.z()) {
    return true;
  } else if (z() > pt.z()) {
    return false;
  } else if (y() < pt.y()) {
    return true;
  } else if (y() > pt.y()) {
    return false;
  }

  return x() < pt.x();
}

void ZPoint::shiftSliceAxis(NeuTube::EAxis axis)
{
  ZGeometry::shiftSliceAxis(m_x, m_y, m_z, axis);
}

void ZPoint::shiftSliceAxisInverse(NeuTube::EAxis axis)
{
  ZGeometry::shiftSliceAxisInverse(m_x, m_y, m_z, axis);
}

double ZPoint::getSliceCoord(NeuTube::EAxis axis) const
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return m_x;
  case NeuTube::Y_AXIS:
    return m_y;
  case NeuTube::Z_AXIS:
    return m_z;
  }

  return m_z;
}
