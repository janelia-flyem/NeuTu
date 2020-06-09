#include "zpoint.h"

#include <iostream>
#include <sstream>
#include <cmath>
#include <stdio.h>
#include <cstdio>

#if defined(_QT_GUI_USED_)
#include <QPainter>
#endif

#include "tz_geo3d_utils.h"
#include "tz_coordinate_3d.h"
#include "tz_geo3d_utils.h"

#include "common/math.h"
#include "zintpoint.h"
#include "zgeometry.h"

const double ZPoint::MIN_DIST = 1e-5;
const ZPoint ZPoint::INVALID_POINT = ZPoint::InvalidPoint();
const ZPoint ZPoint::ORIGIN = ZPoint(0, 0, 0);

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

/*
void ZPoint::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZPoint::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}
*/

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

double ZPoint::lengthSqure() const
{
  return Geo3d_Orgdist_Sqr(m_x, m_y, m_z);
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

ZPoint operator + (const ZPoint &pt, double offset)
{
  return ZPoint(pt.x() + offset, pt.y() + offset, pt.z() + offset);
}

ZPoint operator - (const ZPoint &pt1, const ZPoint &pt2)
{
  return ZPoint(pt1) -= pt2;
}

ZPoint operator - (const ZPoint &pt, double offset)
{
  return ZPoint(pt.x() - offset, pt.y() - offset, pt.z() - offset);
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
  if (!isApproxOrigin() && !isUnitVector()) {
    double len = length();
    m_x /= len;
    m_y /= len;
    m_z /= len;
  }

#if 0
  coordinate_3d_t coord;
  toArray(coord);
  Coordinate_3d_Unitize(coord);
  set(coord[0], coord[1], coord[2]);
#endif

}

ZPoint ZPoint::getNormalized() const
{
  ZPoint pt = *this;
  pt.normalize();

  return pt;
}

const double& ZPoint::operator [](int index) const
{
//  TZ_ASSERT(index >= 0 && index < 3, "Invalid index");

  switch (index) {
  case 0:
    return m_x;
  case 1:
    return m_y;
  case 2:
    return m_z;
  default:
    throw std::invalid_argument("Invalid input index");
//    break;
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

bool ZPoint::operator ==(const ZPoint &pt) const
{
  return this->x() == pt.x() && this->y() == pt.y() && this->z() == pt.z();
}

bool ZPoint::operator !=(const ZPoint &pt) const
{
  return this->x() != pt.x() || this->y() != pt.y() || this->z() != pt.z();
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
  return (length() < MIN_DIST);
}

bool ZPoint::approxEquals(const ZPoint &pt) const
{
  return (distanceTo(pt) < MIN_DIST);
}

bool ZPoint::isUnitVector() const
{
  return std::fabs(length() - 1.0) < MIN_DIST;
}

bool ZPoint::isPendicularTo(const ZPoint &pt) const
{
  double len1 = length();
  double len2 = pt.length();

  if (len1 < MIN_DIST || len2 < MIN_DIST) {
    return false;
  }

  if (std::fabs(dot(pt)) < MIN_DIST * len1 * len2) {
    return true;
  }

  return false;
}

bool ZPoint::isParallelTo(const ZPoint &pt) const
{
  if (isApproxOrigin() || pt.isApproxOrigin()) {
    return false;
  }

  return getNormalized().approxEquals(pt.getNormalized());
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
  return ZIntPoint(neutu::nround(x()), neutu::nround(y()), neutu::nround(z()));
}

bool ZPoint::hasIntCoord() const
{
  return (std::ceil(getX()) == getX()) && (std::ceil(getY()) == getY()) &&
      (std::ceil(getZ()) == getZ());
}

std::vector<double> ZPoint::toArray() const
{
  return std::vector<double>({m_x, m_y, m_z});
}

void ZPoint::rotate(double theta, double psi)
{
  Geo3d_Rotate_Coordinate(&(m_x), &(m_y), &(m_z),
                          theta, psi, _FALSE_);
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

void ZPoint::shiftSliceAxis(neutu::EAxis axis)
{
  zgeom::ShiftSliceAxis(m_x, m_y, m_z, axis);
}

void ZPoint::shiftSliceAxisInverse(neutu::EAxis axis)
{
  zgeom::ShiftSliceAxisInverse(m_x, m_y, m_z, axis);
}

double ZPoint::getSliceCoord(neutu::EAxis axis) const
{
  switch (axis) {
  case neutu::EAxis::X:
    return m_x;
  case neutu::EAxis::Y:
    return m_y;
  case neutu::EAxis::Z:
    return m_z;
  case neutu::EAxis::ARB:
    return 0;
  }

  return m_z;
}

bool ZPoint::isValid() const
{
  return std::isnan(m_x) == false;
}

void ZPoint::invalidate()
{
  m_x = std::numeric_limits<double>::quiet_NaN();
}

ZPoint ZPoint::InvalidPoint()
{
  ZPoint pt;
  pt.invalidate();
  return pt;
}

std::ostream &operator<<(std::ostream &stream, const ZPoint &pt)
{
  stream << "(" << pt.getX() << ", " << pt.getY() << ", " << pt.getZ() << ")";
  return stream;
}
