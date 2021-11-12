#include "geometry/zcuboid.h"

#include <math.h>
#include <cstddef>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <limits>

#include "common/utilities.h"
#include "common/math.h"
#include "zintpoint.h"
#include "zintcuboid.h"
#include "zlinesegment.h"

#ifndef NULL
#  define NULL 0x0
#endif

using namespace std;

ZCuboid::ZCuboid()
{
  m_minCorner.set(std::numeric_limits<double>::infinity());
  m_maxCorner.set(-std::numeric_limits<double>::infinity());
}

ZCuboid::ZCuboid(double x1, double y1, double z1,
                 double x2, double y2, double z2)
{
  set(x1, y1, z1, x2, y2, z2);
}

ZCuboid::ZCuboid(const ZPoint &minCorner, const ZPoint &maxCorner)
{
  set(minCorner, maxCorner);
}

ZCuboid::ZCuboid(const ZCuboid &cuboid) : m_minCorner(cuboid.m_minCorner),
  m_maxCorner(cuboid.m_maxCorner)
{
}

void ZCuboid::set(double x1, double y1, double z1,
                  double x2, double y2, double z2)
{
  m_minCorner.set(x1, y1, z1);
  m_maxCorner.set(x2, y2, z2);
}

void ZCuboid::set(const ZPoint &minCorner, const ZPoint &maxCorner)
{
  m_minCorner = minCorner;
  m_maxCorner = maxCorner;
}

void ZCuboid::set(const ZIntPoint &minCorner, const ZIntPoint &maxCorner)
{
  m_minCorner.set(minCorner.getX(), minCorner.getY(), minCorner.getZ());
  m_maxCorner.set(maxCorner.getX(), maxCorner.getY(), maxCorner.getZ());
}

void ZCuboid::set(const ZIntCuboid &cuboid)
{
  if (!cuboid.isEmpty()) {
    set(cuboid.getMinCorner().toPoint(),
        cuboid.getMaxCorner().toPoint() + 1.0);
  }
}

void ZCuboid::set(const double *corner)
{
  set(corner[0], corner[1], corner[2], corner[3], corner[4], corner[5]);
}

void ZCuboid::setMinCorner(const ZPoint &pt)
{
  m_minCorner = pt;
}

void ZCuboid::setMinCorner(double x, double y, double z)
{
  m_minCorner.set(x, y, z);
}

void ZCuboid::setMaxCorner(const ZPoint &pt)
{
  m_maxCorner = pt;
}

void ZCuboid::setMaxCorner(double x, double y, double z)
{
  m_maxCorner.set(x, y, z);
}


void ZCuboid::setSize(double width, double height, double depth)
{
  m_maxCorner = m_minCorner + ZPoint(width, height, depth);
}

bool ZCuboid::isValid() const
{
  return (m_maxCorner.x() > m_minCorner.x()) &&
      (m_maxCorner.y() > m_minCorner.y()) &&
      (m_maxCorner.z() > m_minCorner.z());
}

void ZCuboid::invalidate()
{
  m_minCorner.setX(m_maxCorner.x());
}

double ZCuboid::width() const
{
  return (m_maxCorner.x() - m_minCorner.x());
}

double ZCuboid::height() const
{
  return (m_maxCorner.y() - m_minCorner.y());
}

double ZCuboid::depth() const
{
  return (m_maxCorner.z() - m_minCorner.z());
}

double ZCuboid::volume() const
{
  return width() * height() * depth();
}

void ZCuboid::intersect(const ZCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_minCorner[i] = std::max(m_minCorner[i], cuboid.m_minCorner[i]);
    m_maxCorner[i] = std::min(m_maxCorner[i], cuboid.m_maxCorner[i]);
  }
}

void ZCuboid::bind(const ZCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_minCorner[i] = std::min(m_minCorner[i], cuboid.m_minCorner[i]);
    m_maxCorner[i] = std::max(m_maxCorner[i], cuboid.m_maxCorner[i]);
  }
}

double ZCuboid::moveOutFrom(ZCuboid &cuboid, double margin)
{
  double minOffset = -1;
  int movingDim = -1;

  for (int i = 0; i < 3; i++) {
    double offset = cuboid.m_minCorner[i] - m_maxCorner[i];
    if (movingDim < 0) {
      movingDim = 0;
      minOffset = offset;
    } else {
      if (fabs(minOffset) > fabs(offset)) {
        minOffset = offset;
        movingDim = i;
      }
    }

    offset = cuboid.m_maxCorner[i] - m_minCorner[i];
    if (fabs(minOffset) > fabs(offset)) {
      minOffset = offset;
      movingDim = i;
    }
  }

  if (minOffset >= 0.0) {
    minOffset += margin;
  } else {
    minOffset -= margin;
  }

  m_minCorner[movingDim] += minOffset;
  m_maxCorner[movingDim] += minOffset;

  return fabs(minOffset);
}

void ZCuboid::layout(std::vector<ZCuboid> *cuboidArray, double margin)
{
  if (cuboidArray == NULL) {
    return;
  }

  ZCuboid b0 = *this;
  size_t numberOfBoxMoved = 0;

  vector<bool> moved(cuboidArray->size(), false);

  while (numberOfBoxMoved < cuboidArray->size()) {
    double minOffset = -1.0;
    int movingBoxIndex = -1;

    for (size_t i = 0; i < cuboidArray->size(); i++) {
      if (!moved[i]) {
        ZCuboid currentBox = (*cuboidArray)[i];
        double offset = currentBox.moveOutFrom(b0, margin);
        if ((minOffset < 0.0) || (offset < minOffset)) {
          minOffset = offset;
          movingBoxIndex = i;
        }
      }
    }

    (*cuboidArray)[movingBoxIndex].moveOutFrom(b0, margin);
    b0.bind((*cuboidArray)[movingBoxIndex]);
    moved[movingBoxIndex] = true;
    numberOfBoxMoved++;
  }
}

ZCuboid& ZCuboid::operator= (const ZCuboid &cuboid)
{
  m_minCorner = cuboid.m_minCorner;
  m_maxCorner = cuboid.m_maxCorner;

  return *this;
}

double& ZCuboid::operator [](int index)
{
  return const_cast<double&>(static_cast<const ZCuboid&>(*this)[index]);
}

const double& ZCuboid::operator [](int index) const
{
  if (index < 3) {
    return m_minCorner[index];
  }

  return m_maxCorner[index - 3];
}

double ZCuboid::estimateSeparateScale(const ZCuboid &cuboid, const ZPoint &vec)
const
{
  double bestScale = 0.0;

  if (!vec.isApproxOrigin()) {
    //For each dimension
    for (int i = 0; i < 3; i++) {
      //If its moving step is positive
      if (vec[i] > ZPoint::minimalDistance()) {
        //Calculate forwared moving scale
        double diff = cuboid[i] - (*this)[i];
        if (fabs(diff) > ZPoint::minimalDistance()) {
          double scale = 0.0;
          if (diff > 0.0) {
            scale = ((*this)[i+3] - (*this)[i]) / diff / vec[i];
          } else {
            scale = (cuboid[i] - cuboid[i+3]) / diff / vec[i];
          }

          if ((scale < bestScale) || (bestScale == 0.0)) {
            bestScale = scale;
          }
        }
      }
    }
  }

  if (bestScale == 0.0) {
    bestScale = 1.0;
  }

  return bestScale;
}

void ZCuboid::print()
{
  cout << "(" << m_minCorner.x() << "," << m_minCorner.y() << "," <<
          m_minCorner.z() << ")" << " -> (" << m_maxCorner.x() << "," <<
          m_maxCorner.y() << "," << m_maxCorner.z() << ")" << endl;
}

void ZCuboid::scale(double s)
{
  m_minCorner *= s;
  m_maxCorner *= s;
}

void ZCuboid::scale(double sx, double sy, double sz)
{
  m_minCorner *= ZPoint(sx, sy, sz);
  m_maxCorner *= ZPoint(sx, sy, sz);
}

void ZCuboid::expand(double margin)
{
  m_minCorner -= margin;
  m_maxCorner += margin;
}

ZPoint ZCuboid::getCorner(int index) const
{
//  TZ_ASSERT(index >= 0 && index <= 7, "invalid index.");

  switch (index) {
  case 0:
    return m_minCorner;
  case 7:
    return m_maxCorner;
  case 1:
    return ZPoint(m_maxCorner.x(), m_minCorner.y(), m_minCorner.z());
  case 2:
    return ZPoint(m_minCorner.x(), m_maxCorner.y(), m_minCorner.z());
  case 3:
    return ZPoint(m_maxCorner.x(), m_maxCorner.y(), m_minCorner.z());
  case 4:
    return ZPoint(m_minCorner.x(), m_minCorner.y(), m_maxCorner.z());
  case 5:
    return ZPoint(m_maxCorner.x(), m_minCorner.y(), m_maxCorner.z());
  case 6:
    return ZPoint(m_minCorner.x(), m_maxCorner.y(), m_maxCorner.z());
  default:
    throw std::invalid_argument("Invalid corner index");
//    break;
  }

  return ZPoint(0, 0, 0);
}

ZPoint ZCuboid::getCenter() const
{
  return ZPoint((m_minCorner.x() + m_maxCorner.x()) / 2.0,
                (m_minCorner.y() + m_maxCorner.y()) / 2.0,
                (m_minCorner.z() + m_maxCorner.z()) / 2.0);
}

void ZCuboid::join(double x, double y, double z)
{
  joinX(x);
  joinY(y);
  joinZ(z);
}

void ZCuboid::join(const ZPoint &pt)
{
  join(pt.getX(), pt.getY(), pt.getZ());
}

void ZCuboid::joinX(double x)
{
  if (m_minCorner.x() > x) {
    m_minCorner.setX(x);
  } else if (m_maxCorner.x() < x) {
    m_maxCorner.setX(x);
  }
}

void ZCuboid::joinY(double y)
{
  if (m_minCorner.y() > y) {
    m_minCorner.setY(y);
  } else if (m_maxCorner.y() < y) {
    m_maxCorner.setY(y);
  }
}

void ZCuboid::joinZ(double z)
{
  if (m_minCorner.z() > z) {
    m_minCorner.setZ(z);
  } else if (m_maxCorner.z() < z) {
    m_maxCorner.setZ(z);
  }
}

void ZCuboid::join(const ZCuboid &box)
{
  if (box.isValid()) {
    include(box.getMinCorner());
    include(box.getMaxCorner());
  }
}

void ZCuboid::join(const ZIntCuboid &box)
{
  if (!box.isEmpty()) {
    join(FromIntCuboid(box));
  }
}

double ZCuboid::computeDistance(
    double minX1, double maxX1, double minX2, double maxX2)
{
  double dist = 0.0;
  //on the right
  if (maxX1 < minX2) {
    dist = minX2 - maxX1;
  } else if (minX1 > maxX2) {
    dist = minX1 - maxX2;
  }

  return dist;
}

double ZCuboid::computeDistance(const ZCuboid &box) const
{
  double xDist = computeDistance(getMinCorner().x(), getMaxCorner().x(),
                                 box.getMinCorner().x(), box.getMaxCorner().x());
  double yDist = computeDistance(getMinCorner().y(), getMaxCorner().y(),
                                 box.getMinCorner().y(), box.getMaxCorner().y());
  double zDist = computeDistance(getMinCorner().z(), getMaxCorner().z(),
                                 box.getMinCorner().z(), box.getMaxCorner().z());

  return sqrt(xDist * xDist + yDist * yDist + zDist * zDist);
}

void ZCuboid::include(const ZPoint &point) {
  joinX(point.x());
  joinY(point.y());
  joinZ(point.z());
}

void ZCuboid::translate(const ZPoint &pt)
{
  m_minCorner += pt;
  m_maxCorner += pt;
}

#define SET_HIT_POINT(x, y, z) \
{ \
  ZPoint hitPoint(x, y, z);\
  if (hitCount == 0) {\
    segBuffer.setStartPoint(hitPoint);\
  } else if (hitCount == 1) {\
    segBuffer.setEndPoint(hitPoint);\
  } else if (segBuffer.getLength() <\
             segBuffer.getStartPoint().distanceTo(hitPoint)) {\
    segBuffer.setEndPoint(hitPoint);\
  }\
  ++hitCount;\
}

bool ZCuboid::intersectLine(
    const ZPoint &p0, const ZPoint &slope, ZLineSegment *seg) const
{
  if (slope.x() == 0.0 && slope.y() == 0.0 && slope.z() == 0.0) {
    return false;
  }

  ZPoint firstCorner = m_minCorner - p0;
  ZPoint lastCorner = m_maxCorner - p0;

  //For a line parallel to an axis
  //parallel to X
  if (slope.y() == 0 && slope.z() == 0) {
    if (neutu::WithinCloseRange(0.0, firstCorner.y(), lastCorner.y()) &&
        neutu::WithinCloseRange(0.0, firstCorner.z(), lastCorner.z())) {
      if (seg != NULL) {
        seg->setStartPoint(m_minCorner.x(), p0.y(), p0.z());
        seg->setEndPoint(m_maxCorner.x(), p0.y(), p0.z());
      }
      return true;
    }
  }

  //parallel to Y
  if (slope.x() == 0 && slope.z() == 0) {
    if (neutu::WithinCloseRange(0.0, firstCorner.x(), lastCorner.x()) &&
        neutu::WithinCloseRange(0.0, firstCorner.z(), lastCorner.z())) {
      if (seg != NULL) {
        seg->setStartPoint(p0.x(), m_minCorner.y(), p0.z());
        seg->setEndPoint(p0.x(), m_maxCorner.y(), p0.z());
      }
      return true;
    }
  }

  //parallel to Z
  if (slope.x() == 0 && slope.y() == 0) {
    if (neutu::WithinCloseRange(0.0, firstCorner.x(), lastCorner.x()) &&
        neutu::WithinCloseRange(0.0, firstCorner.y(), lastCorner.y())) {
      if (seg != NULL) {
        seg->setStartPoint(p0.x(), p0.y(), m_minCorner.z());
        seg->setEndPoint(p0.x(), p0.y(), m_maxCorner.z());
      }
      return true;
    }
  }

  ZLineSegment segBuffer;

  //For a line parallel to a face
  //parallel to X-Y
  if (slope.z() == 0) {
    if (neutu::WithinCloseRange(0.0, firstCorner.z(), lastCorner.z())) {
      //face x0
      double t = firstCorner.x() / slope.x();
      double y = t * slope.y();
      //double z = 0;
      int hitCount = 0;
      if (neutu::WithinCloseRange(y, firstCorner.y(), lastCorner.y())) {
        segBuffer.setStartPoint(m_minCorner.x(), p0.y() + y, p0.z());
        /*
        if (seg != NULL) {
          seg->setStartPoint(m_firstCorner.x(), p0.y() + y, p0.z());
        }
        */
        ++hitCount;
      }

      //face x1
      t = lastCorner.x() / slope.x();
      y = t * slope.y();
      if (neutu::WithinCloseRange(y, firstCorner.y(), lastCorner.y())) {
        SET_HIT_POINT(m_maxCorner.x(), p0.y() + y, p0.z());
      }

      //face y0
      t = firstCorner.y() / slope.y();
      double x = t * slope.x();
      if (neutu::WithinCloseRange(x, firstCorner.x(), lastCorner.x())) {
        SET_HIT_POINT(p0.x() + x, m_minCorner.y(), p0.z());
      }

      //face y1
      t = lastCorner.y() / slope.y();
      x = t * slope.x();
      if (neutu::WithinCloseRange(x, firstCorner.x(), lastCorner.x())) {
        SET_HIT_POINT(p0.x() + x, m_maxCorner.y(), p0.z());
      }
      if (hitCount > 1) {
        if (seg != NULL) {
          seg->set(segBuffer.getStartPoint(), segBuffer.getEndPoint());
        }
        return true;
      }
    }
  }

  //parallel to X-Z
  if (slope.y() == 0) {
    if (neutu::WithinCloseRange(0.0, firstCorner.y(), lastCorner.y())) {
      //face x0
      double t = firstCorner.x() / slope.x();
      double z = t * slope.z();
      int hitCount = 0;
      if (neutu::WithinCloseRange(z, firstCorner.z(), lastCorner.z())) {
        SET_HIT_POINT(m_minCorner.x(), p0.y(), p0.z() + z);
      }

      //face x1
      t = lastCorner.x() / slope.x();
      z = t * slope.z();
      if (neutu::WithinCloseRange(z, firstCorner.z(), lastCorner.z())) {
        SET_HIT_POINT(m_maxCorner.x(), p0.y(), p0.z() + z);
      }

      //face z0
      t = firstCorner.z() / slope.z();
      double x = t * slope.x();
      if (neutu::WithinCloseRange(x, firstCorner.x(), lastCorner.x())) {
        SET_HIT_POINT(p0.x() + x, p0.y(), m_minCorner.z());
      }

      //face y1
      t = lastCorner.z() / slope.z();
      x = t * slope.x();
      if (neutu::WithinCloseRange(x, firstCorner.x(), lastCorner.x())) {
        SET_HIT_POINT(p0.x() + x, p0.y(), m_maxCorner.z());
      }
      if (hitCount > 1) {
        if (seg != NULL) {
          *seg = segBuffer;
        }
        return true;
      }
    }
  }

  //parallel to Y-Z
  if (slope.x() == 0) {
    if (neutu::WithinCloseRange(0.0, firstCorner.x(), lastCorner.x())) {
      //face y0
      double t = firstCorner.y() / slope.y();
      double z = t * slope.z();
      int hitCount = 0;
      if (neutu::WithinCloseRange(z, firstCorner.z(), lastCorner.z())) {
        SET_HIT_POINT(p0.x(), m_minCorner.y(), p0.z() + z);
      }

      //face y1
      t = lastCorner.y() / slope.y();
      z = t * slope.z();
      if (neutu::WithinCloseRange(z, firstCorner.z(), lastCorner.z())) {
        SET_HIT_POINT(p0.x(), m_maxCorner.y(), p0.z() + z);
      }

      //face z0
      t = firstCorner.z() / slope.z();
      double y = t * slope.y();
      if (neutu::WithinCloseRange(y, firstCorner.y(), lastCorner.y())) {
        SET_HIT_POINT(p0.x(), p0.y() + y, m_minCorner.z());
      }

      //face z1
      t = lastCorner.z() / slope.z();
      y = t * slope.y();
      if (neutu::WithinCloseRange(y, firstCorner.y(), lastCorner.y())) {
        SET_HIT_POINT(p0.x(), p0.y() + y, m_maxCorner.z());
      }

      if (hitCount > 1) {
        if (seg != NULL) {
          seg->set(segBuffer.getStartPoint(), segBuffer.getEndPoint());
        }
        return true;
      }
    }
  }

  //Non-parallel case
  //face x0
  double t = firstCorner.x() / slope.x();
  double y = t * slope.y();
  double z = t * slope.z();
  int hitCount = 0;
  if (neutu::WithinCloseRange(y, firstCorner.y(), lastCorner.y()) &&
      neutu::WithinCloseRange(z, firstCorner.z(), lastCorner.z())) {
    SET_HIT_POINT(m_minCorner.x(), p0.y() + y, p0.z() + z);
  }

  //face x1
  t = lastCorner.x() / slope.x();
  y = t * slope.y();
  z = t * slope.z();
  if (neutu::WithinCloseRange(y, firstCorner.y(), lastCorner.y()) &&
      neutu::WithinCloseRange(z, firstCorner.z(), lastCorner.z())) {
    SET_HIT_POINT(m_maxCorner.x(), p0.y() + y, p0.z() + z);
  }

  //face y0
  t = firstCorner.y() / slope.y();
  double x = t * slope.x();
  z = t * slope.z();
  if (neutu::WithinCloseRange(x, firstCorner.x(), lastCorner.x()) &&
      neutu::WithinCloseRange(z, firstCorner.z(), lastCorner.z())) {
    SET_HIT_POINT(p0.x() + x, m_minCorner.y(), p0.z() + z);
  }

  //face y1
  t = lastCorner.y() / slope.y();
  x = t * slope.x();
  z = t * slope.z();
  if (neutu::WithinCloseRange(x, firstCorner.x(), lastCorner.x()) &&
      neutu::WithinCloseRange(z, firstCorner.z(), lastCorner.z())) {
    SET_HIT_POINT(p0.x() + x, m_maxCorner.y(), p0.z() + z);
  }


  //face z0
  t = firstCorner.z() / slope.z();
  x = t * slope.x();
  y = t * slope.y();
  if (neutu::WithinCloseRange(x, firstCorner.x(), lastCorner.x()) &&
      neutu::WithinCloseRange(y, firstCorner.y(), lastCorner.y())) {
    SET_HIT_POINT(p0.x() + x, p0.y() + y, m_minCorner.z());
  }

  //face z1
  t = lastCorner.z() / slope.z();
  x = t * slope.x();
  y = t * slope.y();
  if (neutu::WithinCloseRange(x, firstCorner.x(), lastCorner.x()) &&
      neutu::WithinCloseRange(y, firstCorner.y(), lastCorner.y())) {
    SET_HIT_POINT(p0.x() + x, p0.y() + y, m_maxCorner.z());
  }
  if (hitCount > 1) {
    if (seg != NULL) {
      *seg = segBuffer;
    }
    return true;
  }

  return false;
}

ZIntCuboid ZCuboid::toIntCuboid() const
{
  ZIntCuboid cuboid;
  cuboid.setMinCorner(ZIntPoint(neutu::ifloor(m_minCorner.getX()),
                                neutu::ifloor(m_minCorner.getY()),
                                neutu::ifloor(m_minCorner.getZ())));
  int x = neutu::ifloor(m_maxCorner.getX());
  int y = neutu::ifloor(m_maxCorner.getY());
  int z = neutu::ifloor(m_maxCorner.getZ());
  if (double(x) == m_maxCorner.getX()) {
    --x;
  }
  if (double(y) == m_maxCorner.getY()) {
    --y;
  }
  if (double(z) == m_maxCorner.getZ()) {
    --z;
  }
  cuboid.setMaxCorner(ZIntPoint(x, y, z));

  return cuboid;
}

ZCuboid ZCuboid::FromIntCuboid(const ZIntCuboid &cuboid)
{
  ZCuboid result;
  result.set(cuboid);
  return result;
}

double ZCuboid::getDiagonalLength() const
{
  return sqrt(width() * width() + height() * height() + depth() * depth());
}

int ZCuboid::getMinSideLength() const
{
  return std::min(std::min(width(), height()), depth());
}

std::vector<double> ZCuboid::toCornerVector() const
{
  std::vector<double> corner(6);
  corner[0] = getMinCorner().x();
  corner[1] = getMaxCorner().x();

  corner[2] = getMinCorner().y();
  corner[3] = getMaxCorner().y();

  corner[4] = getMinCorner().z();
  corner[5] = getMaxCorner().z();

  return corner;
}

bool ZCuboid::contains(const ZPoint &pt) const
{
  ZPoint minCorner = getMinCorner();
  ZPoint maxCorner = getMaxCorner();

  return neutu::WithinCloseRange(pt.x(), minCorner.x(), maxCorner.x()) &&
      neutu::WithinCloseRange(pt.y(), minCorner.y(), maxCorner.y()) &&
      neutu::WithinCloseRange(pt.z(), minCorner.z(), maxCorner.z());
}

bool ZCuboid::contains(const ZCuboid &box) const
{
  return contains(box.getMinCorner()) && contains(box.getMaxCorner());
}

ZCuboid ZCuboid::MakeFromCorner(const ZPoint &c1, const ZPoint &c2)
{
  ZCuboid box;
  box.setMinCorner({std::min(c1.getX(), c2.getX()),
                    std::min(c1.getY(), c2.getY()),
                    std::min(c1.getZ(), c2.getZ())});
  box.setMaxCorner({std::max(c1.getX(), c2.getX()),
                    std::max(c1.getY(), c2.getY()),
                    std::max(c1.getZ(), c2.getZ())});

  return box;
}
