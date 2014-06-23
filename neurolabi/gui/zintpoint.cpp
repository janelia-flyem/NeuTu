#include "zintpoint.h"
#include <iostream>
#include "tz_error.h"
#include "zerror.h"

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

ZIntPoint operator + (const ZIntPoint &pt1, const ZIntPoint &pt2)
{
  return ZIntPoint(pt1.getX() + pt2.getX(), pt1.getY() + pt2.getY(),
                   pt1.getZ() + pt2.getZ());
}
