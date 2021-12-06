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
  m_minCorner.set(x1, y1, z1);
  m_maxCorner.set(x2, y2, z2);
}

ZIntCuboid::ZIntCuboid(const Cuboid_I &cuboid)
{
  m_minCorner.set(cuboid.cb[0], cuboid.cb[1], cuboid.cb[2]);
  m_maxCorner.set(cuboid.ce[0], cuboid.ce[1], cuboid.ce[2]);
}

ZIntCuboid::ZIntCuboid(
    const ZIntPoint &firstCorner, const ZIntPoint &lastCorner)
{
  m_minCorner = firstCorner;
  m_maxCorner = lastCorner;
}

void ZIntCuboid::reset()
{
  m_minCorner.set(0, 0, 0);
  m_maxCorner.set(-1, -1, -1);
}

int ZIntCuboid::getWidth() const
{
  return m_maxCorner.getX() - m_minCorner.getX() + 1;
}

int ZIntCuboid::getHeight() const
{
  return m_maxCorner.getY() - m_minCorner.getY() + 1;
}

int ZIntCuboid::getDepth() const
{
  return m_maxCorner.getZ() - m_minCorner.getZ() + 1;
}

ZIntPoint ZIntCuboid::getSize() const
{
  return ZIntPoint(getWidth(), getHeight(), getDepth());
}

double ZIntCuboid::getDiagonalLength() const
{
  if (isEmpty()) {
    return 0;
  }

  return ZPoint(getWidth(), getHeight(), getDepth()).length();
}

double ZIntCuboid::getMinSideLength() const
{
  return std::min(std::min(getWidth(), getHeight()), getDepth());
}

void ZIntCuboid::setSize(int width, int height, int depth)
{
  m_maxCorner.set(m_minCorner.getX() + width - 1,
                   m_minCorner.getY() + height - 1,
                   m_minCorner.getZ() + depth - 1);
}

void ZIntCuboid::setSize(const ZIntPoint &size)
{
  setSize(size.getX(), size.getY(), size.getZ());
}

void ZIntCuboid::setWidth(int width)
{
  m_maxCorner.setX(m_minCorner.getX() + width - 1);
}

void ZIntCuboid::setHeight(int height)
{
  m_maxCorner.setY(m_minCorner.getY() + height - 1);
}

void ZIntCuboid::set(int x1, int y1, int z1, int x2, int y2, int z2)
{
  setMinCorner(x1, y1, z1);
  setMaxCorner(x2, y2, z2);
}

void ZIntCuboid::set(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner)
{
  setMinCorner(firstCorner);
  setMaxCorner(lastCorner);
}

void ZIntCuboid::translateX(int dx)
{
  m_minCorner.setX(m_minCorner.getX() + dx);
  m_maxCorner.setX(m_maxCorner.getX() + dx);
}

void ZIntCuboid::translate(const ZIntPoint &offset)
{
  m_minCorner += offset;
  m_maxCorner += offset;
}

void ZIntCuboid::translate(int dx, int dy, int dz)
{
  translate(ZIntPoint(dx, dy, dz));
}

void ZIntCuboid::scaleUp(const ZIntPoint &s)
{
  ZIntPoint dim(getWidth(), getHeight(), getDepth());

  m_minCorner *= s;
  m_maxCorner = m_minCorner + dim * s - 1;
}


void ZIntCuboid::scaleUp(int s)
{
  scaleUp(ZIntPoint(s, s, s));
}

void ZIntCuboid::scaleDown(const ZIntPoint &s)
{
  if (s.definitePositive()) {
    m_minCorner /= s;
    m_maxCorner /= s;
  }
}

void ZIntCuboid::scaleDown(int s)
{
  if (s > 0) {
    m_minCorner /= s;
    m_maxCorner /= s;
  }
}

void ZIntCuboid::scaleDownBlock(int s)
{
  if (s > 0) {
    m_minCorner /= s;
    m_maxCorner.setX(m_maxCorner.getX() / s + m_maxCorner.getX() % s);
    m_maxCorner.setY(m_maxCorner.getY() / s + m_maxCorner.getY() % s);
    m_maxCorner.setZ(m_maxCorner.getZ() / s + m_maxCorner.getZ() % s);
  }
}

void ZIntCuboid::scaleDownBlock(const ZIntPoint &s)
{
  if (s.definitePositive()) {
    m_minCorner /= s;
    m_maxCorner.setX(
          m_maxCorner.getX() / s.getX() + m_maxCorner.getX() % s.getX());
    m_maxCorner.setY(
          m_maxCorner.getY() / s.getY() + m_maxCorner.getY() % s.getY());
    m_maxCorner.setZ(
          m_maxCorner.getZ() / s.getZ() + m_maxCorner.getZ() % s.getZ());
  }
}

ZIntCuboid &ZIntCuboid::join(const ZIntCuboid &cuboid)
{
  if (!cuboid.isEmpty()) {
    if (isEmpty()) {
      *this = cuboid;
    } else  {
      for (int i = 0; i < 3; i++) {
        m_minCorner[i] = imin2(m_minCorner[i], cuboid.m_minCorner[i]);
        m_maxCorner[i] = imax2(m_maxCorner[i], cuboid.m_maxCorner[i]);
      }
    }
  }

  return *this;
}

void ZIntCuboid::join(int x, int y, int z)
{
  if (isEmpty()) {
    setMinCorner(x, y, z);
    setMaxCorner(x, y, z);
  } else {
    joinX(x);
    joinY(y);
    joinZ(z);
  }
}

void ZIntCuboid::join(const ZIntPoint &pt)
{
  join(pt.getX(), pt.getY(), pt.getZ());
}

void ZIntCuboid::join(const ZPoint &pt)
{
  join(pt.roundToIntPoint());
}

ZIntCuboid &ZIntCuboid::intersect(const ZIntCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_minCorner[i] = imax2(m_minCorner[i], cuboid.m_minCorner[i]);
    m_maxCorner[i] = imin2(m_maxCorner[i], cuboid.m_maxCorner[i]);
  }

  return *this;
}

void ZIntCuboid::joinX(int x)
{
  if (x < m_minCorner.getX()) {
    m_minCorner.setX(x);
  } else if (x > m_maxCorner.getX()) {
    m_maxCorner.setX(x);
  }
}

void ZIntCuboid::expandX(int dx)
{
  m_minCorner.setX(m_minCorner.getX() - dx);
  m_maxCorner.setX(m_maxCorner.getX() + dx);
}

void ZIntCuboid::expandY(int dy)
{
  m_minCorner.setY(m_minCorner.getY() - dy);
  m_maxCorner.setY(m_maxCorner.getY() + dy);
}

void ZIntCuboid::expandZ(int dz)
{
  m_minCorner.setZ(m_minCorner.getZ() - dz);
  m_maxCorner.setZ(m_maxCorner.getZ() + dz);
}

void ZIntCuboid::expand(int dx, int dy, int dz)
{
  expandX(dx);
  expandY(dy);
  expandZ(dz);
}

void ZIntCuboid::joinY(int y)
{
  if (y < m_minCorner.getY()) {
    m_minCorner.setY(y);
  } else if (y > m_maxCorner.getY()) {
    m_maxCorner.setY(y);
  }
}

void ZIntCuboid::joinZ(int z)
{
  if (z < m_minCorner.getZ()) {
    m_minCorner.setZ(z);
  } else if (z > m_maxCorner.getZ()) {
    m_maxCorner.setZ(z);
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
  ZIntPoint newSize = m_maxCorner / s - m_minCorner / s + 1;

  size_t area = newSize.getX() * newSize.getY();

  return area * newSize.getZ();
}

bool ZIntCuboid::contains(int x, int y, int z) const
{
  return IS_IN_CLOSE_RANGE(x, m_minCorner.getX(), m_maxCorner.getX()) &&
      IS_IN_CLOSE_RANGE(y, m_minCorner.getY(), m_maxCorner.getY()) &&
      IS_IN_CLOSE_RANGE(z, m_minCorner.getZ(), m_maxCorner.getZ());
}

bool ZIntCuboid::contains(const ZIntPoint &pt) const
{
  return contains(pt.getX(), pt.getY(), pt.getZ());
}

bool ZIntCuboid::contains(const ZIntCuboid &box) const
{
  return contains(box.getMinCorner()) && contains(box.getMaxCorner());
}

bool ZIntCuboid::containsYZ(int y, int z) const
{
  return IS_IN_CLOSE_RANGE(y, m_minCorner.getY(), m_maxCorner.getY()) &&
      IS_IN_CLOSE_RANGE(z, m_minCorner.getZ(), m_maxCorner.getZ());
}

bool ZIntCuboid::isEmpty() const
{
  return getWidth() <= 0 || getHeight() <= 0 || getDepth() <= 0;
}

int ZIntCuboid::getMinX() const
{
  return m_minCorner.getX();
}

int ZIntCuboid::getMaxX() const
{
  return m_maxCorner.getX();
}

int ZIntCuboid::getMinY() const
{
  return m_minCorner.getY();
}

int ZIntCuboid::getMaxY() const
{
  return m_maxCorner.getY();
}

int ZIntCuboid::getMinZ() const
{
  return m_minCorner.getZ();
}

int ZIntCuboid::getMaxZ() const
{
  return m_maxCorner.getZ();
}


void ZIntCuboid::setMinX(int x)
{
  m_minCorner.setX(x);
}

void ZIntCuboid::setMaxX(int x)
{
  m_maxCorner.setX(x);
}

void ZIntCuboid::setMinY(int y)
{
  m_minCorner.setY(y);
}

void ZIntCuboid::setMaxY(int y)
{
  m_maxCorner.setY(y);
}

void ZIntCuboid::setMinZ(int z)
{
  m_minCorner.setZ(z);
}

void ZIntCuboid::setMaxZ(int z)
{
  m_maxCorner.setZ(z);
}

void ZIntCuboid::setDepth(int depth)
{
  m_maxCorner.setZ(m_minCorner.getZ() + depth - 1);
}

void ZIntCuboid::setDepth(int depth, neutu::ERangeReference ref)
{
  if (depth != getDepth()) {
    switch (ref) {
    case neutu::ERangeReference::RANGE_MIN:
      setDepth(depth);
      break;
    case neutu::ERangeReference::RANGE_MAX:
      m_minCorner.setZ(m_maxCorner.getZ() - depth + 1);
      break;
    case neutu::ERangeReference::RANGE_CENTER:
    {
      int cz = m_minCorner.getZ() + getDepth() / 2;
      if (depth <= 0) {
        m_minCorner.setZ(cz);
        m_maxCorner.setZ(cz - 1);
      } else {
        int d1 = depth / 2;
        int d2 = depth - d1 - 1;
        m_minCorner.setZ(cz - d1);
        m_maxCorner.setZ(cz + d2);
      }
    }
      break;
    }
  }
}

bool ZIntCuboid::equals(const ZIntCuboid &cuboid) const
{
  return m_minCorner.equals(cuboid.getMinCorner()) &&
      m_maxCorner.equals(cuboid.getMaxCorner());
}

ZIntPoint ZIntCuboid::getCorner(int index) const
{
  switch (index) {
  case 0:
    return getMinCorner();
  case 1:
    return ZIntPoint(getMaxCorner().getX(),
                     getMinCorner().getY(),
                     getMinCorner().getZ());
  case 2:
    return ZIntPoint(getMinCorner().getX(),
                     getMaxCorner().getY(),
                     getMinCorner().getZ());
  case 3:
    return ZIntPoint(getMaxCorner().getX(),
                     getMaxCorner().getY(),
                     getMinCorner().getZ());
  case 4:
    return ZIntPoint(getMinCorner().getX(),
                     getMinCorner().getY(),
                     getMaxCorner().getZ());
  case 5:
    return ZIntPoint(getMaxCorner().getX(),
                     getMinCorner().getY(),
                     getMaxCorner().getZ());
  case 6:
    return ZIntPoint(getMinCorner().getX(),
                     getMaxCorner().getY(),
                     getMaxCorner().getZ());
  case 7:
    return getMaxCorner();
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


  if (box.getMinCorner().getX() > getMaxCorner().getX() ||
      box.getMaxCorner().getX() < getMinCorner().getX()) {
    return false;
  }

  if (box.getMinCorner().getY() > getMaxCorner().getY() ||
      box.getMaxCorner().getY() < getMinCorner().getY()) {
    return false;
  }

  if (box.getMinCorner().getZ() > getMaxCorner().getZ() ||
      box.getMaxCorner().getZ() < getMinCorner().getZ()) {
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
          getMinCorner().getX(), getMaxCorner().getX(),
          box.getMinCorner().getX(), box.getMaxCorner().getX()),
        ComputeRangeDist(
          getMinCorner().getY(), getMaxCorner().getY(),
          box.getMinCorner().getY(), box.getMaxCorner().getY()),
        ComputeRangeDist(
          getMinCorner().getZ(), getMaxCorner().getZ(),
          box.getMinCorner().getZ(), box.getMaxCorner().getZ()));
}

double ZIntCuboid::computeDistance(const ZIntCuboid &box)
{
  double xDist = ComputeRangeDist(
        getMinCorner().getX(), getMaxCorner().getX(),
        box.getMinCorner().getX(), box.getMaxCorner().getX());
  double yDist = ComputeRangeDist(
        getMinCorner().getY(), getMaxCorner().getY(),
        box.getMinCorner().getY(), box.getMaxCorner().getY());
  double zDist = ComputeRangeDist(
        getMinCorner().getZ(), getMaxCorner().getZ(),
        box.getMinCorner().getZ(), box.getMaxCorner().getZ());

  return sqrt(xDist * xDist + yDist * yDist + zDist * zDist);
}

void ZIntCuboid::shiftSliceAxis(neutu::EAxis axis)
{
  m_minCorner.shiftSliceAxis(axis);
  m_maxCorner.shiftSliceAxis(axis);
}

void ZIntCuboid::shiftSliceAxisInverse(neutu::EAxis axis)
{
  m_minCorner.shiftSliceAxisInverse(axis);
  m_maxCorner.shiftSliceAxisInverse(axis);
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

ZPoint ZIntCuboid::getExactCenter() const
{
  return getMinCorner().toPoint() + getSize().toPoint() * 0.5;
}

ZIntPoint ZIntCuboid::getCenter() const
{
  return getMinCorner() + getSize() / 2;
}

void ZIntCuboid::setCenter(const ZIntPoint &center)
{
  int width = getWidth();
  int height = getHeight();
  int depth = getDepth();

  setMinCorner(center - ZIntPoint(width, height, depth) / 2);
  setSize(width, height, depth);
}

ZJsonArray ZIntCuboid::toJsonArray() const
{
  ZJsonArray json;
  json.append(getMinCorner().getX());
  json.append(getMinCorner().getY());
  json.append(getMinCorner().getZ());

  json.append(getMaxCorner().getX());
  json.append(getMaxCorner().getY());
  json.append(getMaxCorner().getZ());

  return json;
}

void ZIntCuboid::loadJson(const ZJsonArray &json)
{
  reset();

  int count = json.size();

  if (count == 6) {
    for (int i = 0; i < count; ++i) {
      setMinCorner(ZJsonParser::integerValue(json.at(0)),
                     ZJsonParser::integerValue(json.at(1)),
                     ZJsonParser::integerValue(json.at(2)));
      setMaxCorner(ZJsonParser::integerValue(json.at(3)),
                    ZJsonParser::integerValue(json.at(4)),
                    ZJsonParser::integerValue(json.at(5)));
    }
  }
}

std::string ZIntCuboid::toString() const
{
  std::ostringstream stream;
  stream << getMinCorner().toString() << "->" << getMaxCorner().toString();
  return stream.str();
}

bool ZIntCuboid::operator ==(const ZIntCuboid &box) const
{
  return m_minCorner == box.m_minCorner && m_maxCorner == box.m_maxCorner;
}

bool ZIntCuboid::operator !=(const ZIntCuboid &box) const
{
  return m_minCorner != box.m_minCorner ||
      m_maxCorner != box.m_maxCorner;
}

ZIntCuboid operator +(const ZIntCuboid &box, const ZIntPoint &pt)
{
  return ZIntCuboid(box.getMinCorner() + pt, box.getMaxCorner() + pt);
}

ZIntCuboid operator -(const ZIntCuboid &box, const ZIntPoint &pt)
{
  return ZIntCuboid(box.getMinCorner() - pt, box.getMaxCorner() - pt);
}

ZIntCuboid operator *(const ZIntCuboid &box, const ZIntPoint &pt)
{
  return ZIntCuboid(box.getMinCorner() * pt, box.getMaxCorner() * pt);
}

void ZIntCuboid::downScale(int sx, int sy, int sz)
{
  if (sx > 0 && sy > 0 && sz > 0) {
    m_minCorner /= ZIntPoint(sx, sy, sz);
    m_maxCorner /= ZIntPoint(sx, sy, sz);
  }
}

void ZIntCuboid::downScale(int s)
{
  if (s > 1) {
    m_minCorner /= s;
    m_maxCorner /= s;
  }
}

std::ostream& operator<< (std::ostream &stream, const ZIntCuboid &box)
{
  return stream << "(" << box.m_minCorner << ", " << box.m_maxCorner << ")";
}

ZIntCuboid ZIntCuboid::Empty()
{
  return {0, 0, 0, -1, -1, -1};
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
