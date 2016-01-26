#include "zintpoint.h"
#include <iostream>
#include "tz_error.h"
#include "zerror.h"
#include "zpoint.h"
#include "tz_geo3d_utils.h"
#include "geometry/zgeometry.h"

ZIntPoint::ZIntPoint() : m_x(0), m_y(0), m_z(0)
{
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
  return ZIntPoint(pt1.getX() + pt2.getX(), pt1.getY() + pt2.getY(),
                   pt1.getZ() + pt2.getZ());
}

ZIntPoint operator + (const ZIntPoint &pt1, int v)
{
  return ZIntPoint(pt1.getX() + v, pt1.getY() + v, pt1.getZ() + v);
}

ZIntPoint operator - (const ZIntPoint &pt1, const ZIntPoint &pt2)
{
  return ZIntPoint(pt1.getX() - pt2.getX(), pt1.getY() - pt2.getY(),
                   pt1.getZ() - pt2.getZ());
}

ZIntPoint operator / (const ZIntPoint &pt1, const ZIntPoint &pt2)
{
  if (pt2.getX() == 0 || pt2.getY() == 0 || pt2.getZ() == 0) {
    return ZIntPoint(0, 0, 0);
  }

  return ZIntPoint(pt1.getX() / pt2.getX(), pt1.getY() / pt2.getY(),
                   pt1.getZ() / pt2.getZ());
}

ZIntPoint operator / (const ZIntPoint &pt1, int scale)
{
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

void ZIntPoint::shiftSliceAxis(NeuTube::EAxis axis)
{
  ZGeometry::shiftSliceAxis(m_x, m_y, m_z, axis);
}

void ZIntPoint::shiftSliceAxisInverse(NeuTube::EAxis axis)
{
  ZGeometry::shiftSliceAxisInverse(m_x, m_y, m_z, axis);
}

int ZIntPoint::getSliceCoord(NeuTube::EAxis axis) const
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
