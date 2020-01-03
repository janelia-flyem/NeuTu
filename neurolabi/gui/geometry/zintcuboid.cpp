#include "zintcuboid.h"
#include <cmath>
#include <sstream>

#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zpoint.h"

ZIntCuboid::ZIntCuboid()
{
  reset();
}

ZIntCuboid::ZIntCuboid(int x1, int y1, int z1, int x2, int y2, int z2)
{
  m_firstCorner.set(x1, y1, z1);
  m_lastCorner.set(x2, y2, z2);
}

ZIntCuboid::ZIntCuboid(const Cuboid_I &cuboid)
{
  m_firstCorner.set(cuboid.cb[0], cuboid.cb[1], cuboid.cb[2]);
  m_lastCorner.set(cuboid.ce[0], cuboid.ce[1], cuboid.ce[2]);
}

ZIntCuboid::ZIntCuboid(
    const ZIntPoint &firstCorner, const ZIntPoint &lastCorner)
{
  m_firstCorner = firstCorner;
  m_lastCorner = lastCorner;
}

void ZIntCuboid::reset()
{
  m_firstCorner.set(0, 0, 0);
  m_lastCorner.set(-1, -1, -1);
}

int ZIntCuboid::getWidth() const
{
  return m_lastCorner.getX() - m_firstCorner.getX() + 1;
}

int ZIntCuboid::getHeight() const
{
  return m_lastCorner.getY() - m_firstCorner.getY() + 1;
}

int ZIntCuboid::getDepth() const
{
  return m_lastCorner.getZ() - m_firstCorner.getZ() + 1;
}

ZIntPoint ZIntCuboid::getSize() const
{
  return ZIntPoint(getWidth(), getHeight(), getDepth());
}

double ZIntCuboid::getDiagonalLength() const
{
  return ZPoint(getWidth(), getHeight(), getDepth()).length();
}

void ZIntCuboid::setSize(int width, int height, int depth)
{
  m_lastCorner.set(m_firstCorner.getX() + width - 1,
                   m_firstCorner.getY() + height - 1,
                   m_firstCorner.getZ() + depth - 1);
}

void ZIntCuboid::setSize(const ZIntPoint &size)
{
  setSize(size.getX(), size.getY(), size.getZ());
}

void ZIntCuboid::setWidth(int width)
{
  m_lastCorner.setX(m_firstCorner.getX() + width - 1);
}

void ZIntCuboid::setHeight(int height)
{
  m_lastCorner.setY(m_firstCorner.getY() + height - 1);
}

void ZIntCuboid::set(int x1, int y1, int z1, int x2, int y2, int z2)
{
  setFirstCorner(x1, y1, z1);
  setLastCorner(x2, y2, z2);
}

void ZIntCuboid::set(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner)
{
  setFirstCorner(firstCorner);
  setLastCorner(lastCorner);
}

void ZIntCuboid::translateX(int dx)
{
  m_firstCorner.setX(m_firstCorner.getX() + dx);
  m_lastCorner.setX(m_lastCorner.getX() + dx);
}

void ZIntCuboid::translate(const ZIntPoint &offset)
{
  m_firstCorner += offset;
  m_lastCorner += offset;
}

void ZIntCuboid::scale(const ZIntPoint &s)
{
  ZIntPoint dim(getWidth(), getHeight(), getDepth());

  m_firstCorner *= s;
  m_lastCorner = m_firstCorner + dim * s - 1;
}


void ZIntCuboid::scale(int s)
{
  scale(ZIntPoint(s, s, s));
}

void ZIntCuboid::scaleDown(const ZIntPoint &s)
{
  if (s.definitePositive()) {
    m_firstCorner /= s;
    m_lastCorner /= s;
  }
}

void ZIntCuboid::scaleDown(int s)
{
  if (s > 0) {
    m_firstCorner /= s;
    m_lastCorner /= s;
  }
}

void ZIntCuboid::scaleDownBlock(int s)
{
  if (s > 0) {
    m_firstCorner /= s;
    m_lastCorner.setX(m_lastCorner.getX() / s + m_lastCorner.getX() % s);
    m_lastCorner.setY(m_lastCorner.getY() / s + m_lastCorner.getY() % s);
    m_lastCorner.setZ(m_lastCorner.getZ() / s + m_lastCorner.getZ() % s);
  }
}

void ZIntCuboid::scaleDownBlock(const ZIntPoint &s)
{
  if (s.definitePositive()) {
    m_firstCorner /= s;
    m_lastCorner.setX(
          m_lastCorner.getX() / s.getX() + m_lastCorner.getX() % s.getX());
    m_lastCorner.setY(
          m_lastCorner.getY() / s.getY() + m_lastCorner.getY() % s.getY());
    m_lastCorner.setZ(
          m_lastCorner.getZ() / s.getZ() + m_lastCorner.getZ() % s.getZ());
  }
}

ZIntCuboid &ZIntCuboid::join(const ZIntCuboid &cuboid)
{
  if (!cuboid.isEmpty()) {
    if (isEmpty()) {
      *this = cuboid;
    } else  {
      for (int i = 0; i < 3; i++) {
        m_firstCorner[i] = imin2(m_firstCorner[i], cuboid.m_firstCorner[i]);
        m_lastCorner[i] = imax2(m_lastCorner[i], cuboid.m_lastCorner[i]);
      }
    }
  }

  return *this;
}

void ZIntCuboid::join(int x, int y, int z)
{
  if (isEmpty()) {
    setFirstCorner(x, y, z);
    setLastCorner(x, y, z);
  } else {
    joinX(x);
    joinY(y);
    joinZ(z);
  }
}

ZIntCuboid &ZIntCuboid::intersect(const ZIntCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_firstCorner[i] = imax2(m_firstCorner[i], cuboid.m_firstCorner[i]);
    m_lastCorner[i] = imin2(m_lastCorner[i], cuboid.m_lastCorner[i]);
  }

  return *this;
}

void ZIntCuboid::joinX(int x)
{
  if (x < m_firstCorner.getX()) {
    m_firstCorner.setX(x);
  } else if (x > m_lastCorner.getX()) {
    m_lastCorner.setX(x);
  }
}

void ZIntCuboid::expandX(int dx)
{
  m_firstCorner.setX(m_firstCorner.getX() - dx);
  m_lastCorner.setX(m_lastCorner.getX() + dx);
}

void ZIntCuboid::expandY(int dy)
{
  m_firstCorner.setY(m_firstCorner.getY() - dy);
  m_lastCorner.setY(m_lastCorner.getY() + dy);
}

void ZIntCuboid::expandZ(int dz)
{
  m_firstCorner.setZ(m_firstCorner.getZ() - dz);
  m_lastCorner.setZ(m_lastCorner.getZ() + dz);
}

void ZIntCuboid::expand(int dx, int dy, int dz)
{
  expandX(dx);
  expandY(dy);
  expandZ(dz);
}

void ZIntCuboid::joinY(int y)
{
  if (y < m_firstCorner.getY()) {
    m_firstCorner.setY(y);
  } else if (y > m_lastCorner.getY()) {
    m_lastCorner.setY(y);
  }
}

void ZIntCuboid::joinZ(int z)
{
  if (z < m_firstCorner.getZ()) {
    m_firstCorner.setZ(z);
  } else if (z > m_lastCorner.getZ()) {
    m_lastCorner.setZ(z);
  }
}

size_t ZIntCuboid::getVolume() const
{
  if (getWidth() <= 0 || getHeight() <= 0 || getDepth() <= 0) {
    return 0;
  }

  size_t area = getWidth() * getHeight();

  return area * getDepth();
}

size_t ZIntCuboid::getDsMaxVolume(int xIntv, int yIntv, int zIntv) const
{
  if (getWidth() <= 0 || getHeight() <= 0 || getDepth() <= 0) {
    return 0;
  }

  ZIntPoint s(xIntv + 1, yIntv + 1, zIntv + 1);
  ZIntPoint newSize = m_lastCorner / s - m_firstCorner / s + 1;

  size_t area = newSize.getX() * newSize.getY();

  return area * newSize.getZ();
}

bool ZIntCuboid::contains(int x, int y, int z) const
{
  return IS_IN_CLOSE_RANGE(x, m_firstCorner.getX(), m_lastCorner.getX()) &&
      IS_IN_CLOSE_RANGE(y, m_firstCorner.getY(), m_lastCorner.getY()) &&
      IS_IN_CLOSE_RANGE(z, m_firstCorner.getZ(), m_lastCorner.getZ());
}

bool ZIntCuboid::contains(const ZIntPoint &pt) const
{
  return contains(pt.getX(), pt.getY(), pt.getZ());
}

bool ZIntCuboid::contains(const ZIntCuboid &box) const
{
  return contains(box.getFirstCorner()) && contains(box.getLastCorner());
}

bool ZIntCuboid::containYZ(int y, int z) const
{
  return IS_IN_CLOSE_RANGE(y, m_firstCorner.getY(), m_lastCorner.getY()) &&
      IS_IN_CLOSE_RANGE(z, m_firstCorner.getZ(), m_lastCorner.getZ());
}

bool ZIntCuboid::isEmpty() const
{
  return getWidth() <= 0 || getHeight() <= 0 || getDepth() <= 0;
}

int ZIntCuboid::getFirstX() const
{
  return m_firstCorner.getX();
}

int ZIntCuboid::getLastX() const
{
  return m_lastCorner.getX();
}

int ZIntCuboid::getFirstY() const
{
  return m_firstCorner.getY();
}

int ZIntCuboid::getLastY() const
{
  return m_lastCorner.getY();
}

int ZIntCuboid::getFirstZ() const
{
  return m_firstCorner.getZ();
}

int ZIntCuboid::getLastZ() const
{
  return m_lastCorner.getZ();
}


void ZIntCuboid::setFirstX(int x)
{
  m_firstCorner.setX(x);
}

void ZIntCuboid::setLastX(int x)
{
  m_lastCorner.setX(x);
}

void ZIntCuboid::setFirstY(int y)
{
  m_firstCorner.setY(y);
}

void ZIntCuboid::setLastY(int y)
{
  m_lastCorner.setY(y);
}

void ZIntCuboid::setFirstZ(int z)
{
  m_firstCorner.setZ(z);
}

void ZIntCuboid::setLastZ(int z)
{
  m_lastCorner.setZ(z);
}

void ZIntCuboid::setDepth(int depth)
{
  m_lastCorner.setZ(m_firstCorner.getZ() + depth - 1);
}


bool ZIntCuboid::equals(const ZIntCuboid &cuboid) const
{
  return m_firstCorner.equals(cuboid.getFirstCorner()) &&
      m_lastCorner.equals(cuboid.getLastCorner());
}

ZIntPoint ZIntCuboid::getCorner(int index) const
{
  switch (index) {
  case 0:
    return getFirstCorner();
  case 1:
    return ZIntPoint(getLastCorner().getX(),
                     getFirstCorner().getY(),
                     getFirstCorner().getZ());
  case 2:
    return ZIntPoint(getFirstCorner().getX(),
                     getLastCorner().getY(),
                     getFirstCorner().getZ());
  case 3:
    return ZIntPoint(getLastCorner().getX(),
                     getLastCorner().getY(),
                     getFirstCorner().getZ());
  case 4:
    return ZIntPoint(getFirstCorner().getX(),
                     getFirstCorner().getY(),
                     getLastCorner().getZ());
  case 5:
    return ZIntPoint(getLastCorner().getX(),
                     getFirstCorner().getY(),
                     getLastCorner().getZ());
  case 6:
    return ZIntPoint(getFirstCorner().getX(),
                     getLastCorner().getY(),
                     getLastCorner().getZ());
  case 7:
    return getLastCorner();
  default:
    break;
  }

  return ZIntPoint(0, 0, 0);
}

bool ZIntCuboid::hasOverlap(const ZIntCuboid &box) const
{
  if (isEmpty() || box.isEmpty()) {
    return false;
  }


  if (box.getFirstCorner().getX() > getLastCorner().getX() ||
      box.getLastCorner().getX() < getFirstCorner().getX()) {
    return false;
  }

  if (box.getFirstCorner().getY() > getLastCorner().getY() ||
      box.getLastCorner().getY() < getFirstCorner().getY()) {
    return false;
  }

  if (box.getFirstCorner().getZ() > getLastCorner().getZ() ||
      box.getLastCorner().getZ() < getFirstCorner().getZ()) {
    return false;
  }

  return true;
}

namespace {

int ComputeRangeDist(int x0, int x1, int y0, int y1)
{
  return imax3(0, x0 - y1, y0 - x1);
}

}

int ZIntCuboid::computeBlockDistance(const ZIntCuboid &box)
{
  return imax3(
        ComputeRangeDist(
          getFirstCorner().getX(), getLastCorner().getX(),
          box.getFirstCorner().getX(), box.getLastCorner().getX()),
        ComputeRangeDist(
          getFirstCorner().getY(), getLastCorner().getY(),
          box.getFirstCorner().getY(), box.getLastCorner().getY()),
        ComputeRangeDist(
          getFirstCorner().getZ(), getLastCorner().getZ(),
          box.getFirstCorner().getZ(), box.getLastCorner().getZ()));
}

double ZIntCuboid::computeDistance(const ZIntCuboid &box)
{
  double xDist = ComputeRangeDist(
        getFirstCorner().getX(), getLastCorner().getX(),
        box.getFirstCorner().getX(), box.getLastCorner().getX());
  double yDist = ComputeRangeDist(
        getFirstCorner().getY(), getLastCorner().getY(),
        box.getFirstCorner().getY(), box.getLastCorner().getY());
  double zDist = ComputeRangeDist(
        getFirstCorner().getZ(), getLastCorner().getZ(),
        box.getFirstCorner().getZ(), box.getLastCorner().getZ());

  return sqrt(xDist * xDist + yDist * yDist + zDist * zDist);
}

void ZIntCuboid::shiftSliceAxis(neutu::EAxis axis)
{
  m_firstCorner.shiftSliceAxis(axis);
  m_lastCorner.shiftSliceAxis(axis);
}

void ZIntCuboid::shiftSliceAxisInverse(neutu::EAxis axis)
{
  m_firstCorner.shiftSliceAxisInverse(axis);
  m_lastCorner.shiftSliceAxisInverse(axis);
}

int ZIntCuboid::getDim(neutu::EAxis axis) const
{
  switch (axis) {
  case neutu::EAxis::X:
    return getWidth();
  case neutu::EAxis::Y:
    return getHeight();
  case neutu::EAxis::Z:
    return getDepth();
  case neutu::EAxis::ARB:
    break;
  }

  return 0;
}

ZIntPoint ZIntCuboid::getCenter() const
{
  return getFirstCorner() +
      ZIntPoint(getWidth() / 2, getHeight() / 2, getDepth() / 2);
}

void ZIntCuboid::setCenter(const ZIntPoint &center)
{
  int width = getWidth();
  int height = getHeight();
  int depth = getDepth();

  setFirstCorner(center - ZIntPoint(width, height, depth) / 2);
  setSize(width, height, depth);
}

ZJsonArray ZIntCuboid::toJsonArray() const
{
  ZJsonArray json;
  json.append(getFirstCorner().getX());
  json.append(getFirstCorner().getY());
  json.append(getFirstCorner().getZ());

  json.append(getLastCorner().getX());
  json.append(getLastCorner().getY());
  json.append(getLastCorner().getZ());

  return json;
}

void ZIntCuboid::loadJson(const ZJsonArray &json)
{
  reset();

  int count = json.size();

  if (count == 6) {
    for (int i = 0; i < count; ++i) {
      setFirstCorner(ZJsonParser::integerValue(json.at(0)),
                     ZJsonParser::integerValue(json.at(1)),
                     ZJsonParser::integerValue(json.at(2)));
      setLastCorner(ZJsonParser::integerValue(json.at(3)),
                    ZJsonParser::integerValue(json.at(4)),
                    ZJsonParser::integerValue(json.at(5)));
    }
  }
}

std::string ZIntCuboid::toString() const
{
  std::ostringstream stream;
  stream << getFirstCorner().toString() << "->" << getLastCorner().toString();
  return stream.str();
}

bool ZIntCuboid::operator ==(const ZIntCuboid &box) const
{
  return m_firstCorner == box.m_firstCorner && m_lastCorner == box.m_lastCorner;
}

bool ZIntCuboid::operator !=(const ZIntCuboid &box) const
{
  return m_firstCorner != box.m_firstCorner ||
      m_lastCorner != box.m_lastCorner;
}

/*
double ZIntCuboid::distanceTo(const ZIntPoint &pt)
{
  if (contains(pt)) {
    return 0.0;
  }

  if (contains(pt.getX(), pt.getY(), getFirstCorner().getZ())) {
    return std::min(std::fabs(pt.getZ() - getFirstCorner().getZ()),
                    std::fabs(pt.getZ() - getLastCorner().getZ()));
  }

  if (contains(pt.getX(), getFirstCorner().getY(), pt.getZ())) {
    return std::min(std::fabs(pt.getY() - getFirstCorner().getY()),
                    std::fabs(pt.getY() - getLastCorner().getY()));
  }

  if (contains(pt.getX(), getFirstCorner().getY(), pt.getZ())) {
    return std::min(std::fabs(pt.getY() - getFirstCorner().getY()),
                    std::fabs(pt.getY() - getLastCorner().getY()));
  }
}
*/
