#include "zintpoint.h"
#include <iostream>
#include <climits>

#include "tz_stdint.h"

#include "tz_error.h"
#include "zerror.h"
#include "zpoint.h"
#include "tz_geo3d_utils.h"
#include "geometry/zgeometry.h"
#include "neutube_def.h"
#include "core/utilities.h"

ZIntPoint::ZIntPoint() : m_x(0), m_y(0), m_z(0)
{
}

bool ZIntPoint::IsNormalDimIndex(int index)
{
  return index >= neutube::DIM_MIN_NORMAL_INDEX;
}

ZIntPoint::ZIntPoint(int x, int y, int z)
{
  set(x, y, z);
}

void ZIntPoint::set(int x, int y, int z)
{
  m_x = x;
  m_y = y;
  m_z = z;
}

const int& ZIntPoint::operator [](int index) const
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

int& ZIntPoint::operator[] (int index)
{
  return const_cast<int&>(static_cast<const ZIntPoint&>(*this)[index]);
}

void ZIntPoint::set(const std::vector<int> &pt)
{
  if (pt.size() == 3) {
    set(pt[0], pt[1], pt[2]);
  } else {
    RECORD_WARNING_UNCOND("Unexpected array size.");
  }
}

bool ZIntPoint::definiteLessThan(const ZIntPoint &pt) const
{
  if (getX() < pt.getX()) {
    return (getY() <= pt.getY()) && (getZ() <= pt.getZ());
  }

  if (getY() < pt.getY()) {
    return (getX() <= pt.getX()) && (getZ() <= pt.getZ());
  }

  if (getZ() < pt.getZ()) {
    return (getY() <= pt.getY()) && (getX() <= pt.getX());
  }

  return false;
}

bool ZIntPoint::definitePositive() const
{
  return getX() > 0 && getY() > 0 && getZ() > 0;
}

bool ZIntPoint::semiDefinitePositive() const
{
  return (getX() > 0 || getY() > 0 || getZ() > 0) &&
      (getX() >= 0 && getY() >= 0 && getZ() >= 0);
}

bool ZIntPoint::operator < (const ZIntPoint &pt) const
{
  if (getZ() < pt.getZ()) {
    return true;
  } else if (getZ() > pt.getZ()) {
    return false;
  } else {
    if (getY() < pt.getY()) {
      return true;
    } else if (getY() > pt.getY()) {
      return false;
    } else {
      if (getX() < pt.getX()) {
        return true;
      }
    }
  }

  return false;
}

bool ZIntPoint::operator ==(const ZIntPoint &pt) const
{
  return getX() == pt.getX() && getY() == pt.getY() && getZ() == pt.getZ();
}

bool ZIntPoint::operator !=(const ZIntPoint &pt) const
{
  return getX() != pt.getX() || getY() != pt.getY() || getZ() != pt.getZ();
}

ZIntPoint operator + (const ZIntPoint &pt1, const ZIntPoint &pt2)
{
  if (!pt1.isValid()) {
    return pt1;
  }
  if (!pt2.isValid()) {
    return pt2;
  }

  return ZIntPoint(pt1.getX() + pt2.getX(), pt1.getY() + pt2.getY(),
                   pt1.getZ() + pt2.getZ());
}

ZIntPoint operator + (const ZIntPoint &pt1, int v)
{
  if (!pt1.isValid()) {
    return pt1;
  }
  if (!ZIntPoint::IsValid(v)) {
    return ZIntPoint(v, v, v);
  }

  return ZIntPoint(pt1.getX() + v, pt1.getY() + v, pt1.getZ() + v);
}

ZIntPoint operator * (const ZIntPoint &pt1, const ZIntPoint &pt2)
{
  if (!pt1.isValid()) {
    return pt1;
  }
  if (!pt2.isValid()) {
    return pt2;
  }

  return ZIntPoint(pt1.getX() * pt2.getX(), pt1.getY() * pt2.getY(),
                   pt1.getZ() * pt2.getZ());
}

ZIntPoint operator * (const ZIntPoint &pt1, int v)
{
  if (!pt1.isValid()) {
    return pt1;
  }
  if (!ZIntPoint::IsValid(v)) {
    return ZIntPoint(v, v, v);
  }

  return ZIntPoint(pt1.getX() * v, pt1.getY() * v, pt1.getZ() * v);
}

ZIntPoint operator - (const ZIntPoint &pt1, int v)
{
  if (!pt1.isValid()) {
    return pt1;
  }
  if (!ZIntPoint::IsValid(v)) {
    return ZIntPoint(v, v, v);
  }

  return ZIntPoint(pt1.getX() - v, pt1.getY() - v, pt1.getZ() - v);
}

ZIntPoint operator - (const ZIntPoint &pt1, const ZIntPoint &pt2)
{
  if (!pt1.isValid()) {
    return pt1;
  }
  if (!pt2.isValid()) {
    return pt2;
  }
  return ZIntPoint(pt1.getX() - pt2.getX(),  pt1.getY() - pt2.getY(),
                   pt1.getZ() - pt2.getZ());
}

ZIntPoint operator / (const ZIntPoint &pt1, const ZIntPoint &pt2)
{
  if (!pt1.isValid()) {
    return pt1;
  }
  if (!pt2.isValid()) {
    return pt2;
  }

  if (pt2.getX() == 0 || pt2.getY() == 0 || pt2.getZ() == 0) {
    return ZIntPoint(0, 0, 0);
  }

  return ZIntPoint(pt1.getX() / pt2.getX(), pt1.getY() / pt2.getY(),
                   pt1.getZ() / pt2.getZ());
}

ZIntPoint operator / (const ZIntPoint &pt1, int scale)
{
  if (!pt1.isValid()) {
    return pt1;
  }
  if (!ZIntPoint::IsValid(scale)) {
    return ZIntPoint(scale, scale, scale);
  }

  if (scale == 0) {
    return ZIntPoint(0, 0, 0);
  }

  return ZIntPoint(pt1.getX() / scale, pt1.getY() / scale,
                   pt1.getZ() / scale);
}

std::string ZIntPoint::toString() const
{
  std::ostringstream stream;
  stream << "(" << getX() << ", " << getY() << ", " << getZ() << ")";

  return stream.str();
}

ZIntPoint ZIntPoint::operator - () const
{
  if (!isValid()) {
    return *this;
  }

  return ZIntPoint(-getX(), -getY(), -getZ());
}

ZPoint ZIntPoint::toPoint() const
{
  return ZPoint(getX(), getY(), getZ());
}

bool ZIntPoint::isZero() const
{
  return (getX() == 0) && (getY() == 0) && (getZ() == 0);
}

bool ZIntPoint::equals(const ZIntPoint &pt) const
{
  return (getX() == pt.getX()) && (getY() == pt.getY()) &&
      (getZ() == pt.getZ());
}

double ZIntPoint::distanceTo(double x, double y, double z) const
{
  return Geo3d_Dist(m_x, m_y, m_z, x, y, z);
}

ZIntPoint& ZIntPoint::operator *=(const ZIntPoint &pt)
{
  if (!pt.isValid()) {
    invalidate();
  }

  if (!isValid()) {
    return *this;
  }

  m_x *= pt.m_x;
  m_y *= pt.m_y;
  m_z *= pt.m_z;

  return *this;
}

ZIntPoint& ZIntPoint::operator /=(const ZIntPoint &pt)
{
  if (!pt.isValid()) {
    invalidate();
  }

  if (!isValid()) {
    return *this;
  }

  m_x /= pt.m_x;
  m_y /= pt.m_y;
  m_z /= pt.m_z;

  return *this;
}

ZIntPoint& ZIntPoint::operator /=(int v)
{
  if (v == 0) {
    invalidate();
  }

  if (!isValid()) {
    return *this;
  }

  m_x /= v;
  m_y /= v;
  m_z /= v;

  return *this;
}

ZIntPoint& ZIntPoint::operator +=(const ZIntPoint &pt)
{
  if (!pt.isValid()) {
    invalidate();
  }

  if (!isValid()) {
    return *this;
  }

  m_x += pt.m_x;
  m_y += pt.m_y;
  m_z += pt.m_z;

  return *this;
}

ZIntPoint& ZIntPoint::operator -=(const ZIntPoint &pt)
{
  if (!pt.isValid()) {
    invalidate();
  }

  if (!isValid()) {
    return *this;
  }

  m_x -= pt.getX();
  m_y -= pt.getY();
  m_z -= pt.getZ();

  return *this;
}

void ZIntPoint::shiftSliceAxis(neutube::EAxis axis)
{
  zgeom::shiftSliceAxis(m_x, m_y, m_z, axis);
}

void ZIntPoint::shiftSliceAxisInverse(neutube::EAxis axis)
{
  zgeom::shiftSliceAxisInverse(m_x, m_y, m_z, axis);
}

int ZIntPoint::getSliceCoord(neutube::EAxis axis) const
{
  switch (axis) {
  case neutube::X_AXIS:
    return m_x;
  case neutube::Y_AXIS:
    return m_y;
  case neutube::Z_AXIS:
    return m_z;
  case neutube::A_AXIS:
    return m_z;
  }

  return m_z;
}

void ZIntPoint::invalidate()
{
  set(INT_MIN, INT_MIN, INT_MIN);
}

bool ZIntPoint::IsValid(int x)
{
  return x != INT_MIN;
}

bool ZIntPoint::isValid() const
{
  return m_x != INT_MIN || m_y != INT_MIN || m_z != INT_MIN;
}

void ZIntPoint::read(std::istream &stream)
{
  neutube::read(stream, m_x);
  neutube::read(stream, m_y);
  neutube::read(stream, m_z);
}

void ZIntPoint::write(std::ostream &stream) const
{
  neutube::write(stream, m_x);
  neutube::write(stream, m_y);
  neutube::write(stream, m_z);
}

std::ostream &operator<<(std::ostream &stream, const ZIntPoint &pt)
{
  stream << "(" << pt.getX() << ", " << pt.getY() << ", " << pt.getZ() << ")";
  return stream;
}
