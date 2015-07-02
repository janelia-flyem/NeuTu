#include "zcuboid.h"

#include <math.h>
#include <cstddef>
#include <iostream>

#include "tz_error.h"
#include "tz_utilities.h"
#include "zintpoint.h"
#include "zintcuboid.h"
#include "zlinesegment.h"

#ifndef NULL
#  define NULL 0x0
#endif

using namespace std;

ZCuboid::ZCuboid() : m_firstCorner(), m_lastCorner()
{
}

ZCuboid::ZCuboid(double x1, double y1, double z1,
                 double x2, double y2, double z2)
{
  set(x1, y1, z1, x2, y2, z2);
}

ZCuboid::ZCuboid(const ZCuboid &cuboid) : m_firstCorner(cuboid.m_firstCorner),
  m_lastCorner(cuboid.m_lastCorner)
{
}

void ZCuboid::set(double x1, double y1, double z1,
                  double x2, double y2, double z2)
{
  m_firstCorner.set(x1, y1, z1);
  m_lastCorner.set(x2, y2, z2);
}

void ZCuboid::set(const ZPoint &firstCorner, const ZPoint &lastCorner)
{
  m_firstCorner = firstCorner;
  m_lastCorner = lastCorner;
}

void ZCuboid::set(const ZIntPoint &firstCorner, const ZIntPoint &lastCorner)
{
  m_firstCorner.set(firstCorner.getX(), firstCorner.getY(), firstCorner.getZ());
  m_lastCorner.set(lastCorner.getX(), lastCorner.getY(), lastCorner.getZ());
}

void ZCuboid::set(const double *corner)
{
  set(corner[0], corner[1], corner[2], corner[3], corner[4], corner[5]);
}

void ZCuboid::setFirstCorner(const ZPoint &pt)
{
  m_firstCorner = pt;
}

void ZCuboid::setFirstCorner(double x, double y, double z)
{
  m_firstCorner.set(x, y, z);
}

void ZCuboid::setLastCorner(const ZPoint &pt)
{
  m_lastCorner = pt;
}

void ZCuboid::setLastCorner(double x, double y, double z)
{
  m_lastCorner.set(x, y, z);
}


void ZCuboid::setSize(double width, double height, double depth)
{
  m_lastCorner = m_firstCorner + ZPoint(width, height, depth);
}

bool ZCuboid::isValid() const
{
  return (m_lastCorner.x() > m_firstCorner.x()) &&
      (m_lastCorner.y() > m_firstCorner.y()) &&
      (m_lastCorner.z() > m_firstCorner.z());
}

void ZCuboid::invalidate()
{
  m_firstCorner.setX(m_lastCorner.x());
}

double ZCuboid::width() const
{
  return (m_lastCorner.x() - m_firstCorner.x());
}

double ZCuboid::height() const
{
  return (m_lastCorner.y() - m_firstCorner.y());
}

double ZCuboid::depth() const
{
  return (m_lastCorner.z() - m_firstCorner.z());
}

double ZCuboid::volume() const
{
  return width() * height() * depth();
}

void ZCuboid::intersect(const ZCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_firstCorner[i] = max(m_firstCorner[i], cuboid.m_firstCorner[i]);
    m_lastCorner[i] = min(m_lastCorner[i], cuboid.m_lastCorner[i]);
  }
}

void ZCuboid::bind(const ZCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_firstCorner[i] = min(m_firstCorner[i], cuboid.m_firstCorner[i]);
    m_lastCorner[i] = max(m_lastCorner[i], cuboid.m_lastCorner[i]);
  }
}

double ZCuboid::moveOutFrom(ZCuboid &cuboid, double margin)
{
  double minOffset = -1;
  int movingDim = -1;

  for (int i = 0; i < 3; i++) {
    double offset = cuboid.m_firstCorner[i] - m_lastCorner[i];
    if (movingDim < 0) {
      movingDim = 0;
      minOffset = offset;
    } else {
      if (fabs(minOffset) > fabs(offset)) {
        minOffset = offset;
        movingDim = i;
      }
    }

    offset = cuboid.m_lastCorner[i] - m_firstCorner[i];
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

  m_firstCorner[movingDim] += minOffset;
  m_lastCorner[movingDim] += minOffset;

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
  m_firstCorner = cuboid.m_firstCorner;
  m_lastCorner = cuboid.m_lastCorner;

  return *this;
}

double& ZCuboid::operator [](int index)
{
  return const_cast<double&>(static_cast<const ZCuboid&>(*this)[index]);
}

const double& ZCuboid::operator [](int index) const
{
  if (index < 3) {
    return m_firstCorner[index];
  }

  return m_lastCorner[index - 3];
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
  cout << "(" << m_firstCorner.x() << "," << m_firstCorner.y() << "," <<
          m_firstCorner.z() << ")" << " -> (" << m_lastCorner.x() << "," <<
          m_lastCorner.y() << "," << m_lastCorner.z() << ")" << endl;
}

void ZCuboid::scale(double s)
{
  m_firstCorner *= s;
  m_lastCorner *= s;
}

void ZCuboid::expand(double margin)
{
  m_firstCorner -= margin;
  m_lastCorner += margin;
}

ZPoint ZCuboid::corner(int index) const
{
  TZ_ASSERT(index >= 0 && index <= 7, "invalid index.");

  switch (index) {
  case 0:
    return m_firstCorner;
  case 7:
    return m_lastCorner;
  case 1:
    return ZPoint(m_lastCorner.x(), m_firstCorner.y(), m_firstCorner.z());
  case 2:
    return ZPoint(m_firstCorner.x(), m_lastCorner.y(), m_firstCorner.z());
  case 3:
    return ZPoint(m_lastCorner.x(), m_lastCorner.y(), m_firstCorner.z());
  case 4:
    return ZPoint(m_firstCorner.x(), m_firstCorner.y(), m_lastCorner.z());
  case 5:
    return ZPoint(m_lastCorner.x(), m_firstCorner.y(), m_lastCorner.z());
  case 6:
    return ZPoint(m_firstCorner.x(), m_lastCorner.y(), m_lastCorner.z());
  default:
    break;
  }

  return ZPoint(0, 0, 0);
}

ZPoint ZCuboid::center() const
{
  return ZPoint((m_firstCorner.x() + m_lastCorner.x()) / 2.0,
                (m_firstCorner.y() + m_lastCorner.y()) / 2.0,
                (m_firstCorner.z() + m_lastCorner.z()) / 2.0);
}

void ZCuboid::joinX(double x)
{
  if (m_firstCorner.x() > x) {
    m_firstCorner.setX(x);
  } else if (m_lastCorner.x() < x) {
    m_lastCorner.setX(x);
  }
}

void ZCuboid::joinY(double y)
{
  if (m_firstCorner.y() > y) {
    m_firstCorner.setY(y);
  } else if (m_lastCorner.y() < y) {
    m_lastCorner.setY(y);
  }
}

void ZCuboid::joinZ(double z)
{
  if (m_firstCorner.z() > z) {
    m_firstCorner.setZ(z);
  } else if (m_lastCorner.z() < z) {
    m_lastCorner.setZ(z);
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
  double xDist = computeDistance(firstCorner().x(), lastCorner().x(),
                                 box.firstCorner().x(), box.lastCorner().x());
  double yDist = computeDistance(firstCorner().y(), lastCorner().y(),
                                 box.firstCorner().y(), box.lastCorner().y());
  double zDist = computeDistance(firstCorner().z(), lastCorner().z(),
                                 box.firstCorner().z(), box.lastCorner().z());

  return sqrt(xDist * xDist + yDist * yDist + zDist * zDist);
}

void ZCuboid::include(const ZPoint &point) {
  joinX(point.x());
  joinY(point.y());
  joinZ(point.z());
}

void ZCuboid::translate(const ZPoint &pt)
{
  m_firstCorner += pt;
  m_lastCorner += pt;
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

  ZPoint firstCorner = m_firstCorner - p0;
  ZPoint lastCorner = m_lastCorner - p0;

  //For a line parallel to an axis
  //parallel to X
  if (slope.y() == 0 && slope.z() == 0) {
    if (IS_IN_CLOSE_RANGE(0.0, firstCorner.y(), lastCorner.y()) &&
        IS_IN_CLOSE_RANGE(0.0, firstCorner.z(), lastCorner.z())) {
      if (seg != NULL) {
        seg->setStartPoint(m_firstCorner.x(), p0.y(), p0.z());
        seg->setEndPoint(m_lastCorner.x(), p0.y(), p0.z());
      }
      return true;
    }
  }

  //parallel to Y
  if (slope.x() == 0 && slope.z() == 0) {
    if (IS_IN_CLOSE_RANGE(0.0, firstCorner.x(), lastCorner.x()) &&
        IS_IN_CLOSE_RANGE(0.0, firstCorner.z(), lastCorner.z())) {
      if (seg != NULL) {
        seg->setStartPoint(p0.x(), m_firstCorner.y(), p0.z());
        seg->setEndPoint(p0.x(), m_lastCorner.y(), p0.z());
      }
      return true;
    }
  }

  //parallel to Z
  if (slope.x() == 0 && slope.y() == 0) {
    if (IS_IN_CLOSE_RANGE(0.0, firstCorner.x(), lastCorner.x()) &&
        IS_IN_CLOSE_RANGE(0.0, firstCorner.y(), lastCorner.y())) {
      if (seg != NULL) {
        seg->setStartPoint(p0.x(), p0.y(), m_firstCorner.z());
        seg->setEndPoint(p0.x(), p0.y(), m_lastCorner.z());
      }
      return true;
    }
  }

  ZLineSegment segBuffer;

  //For a line parallel to a face
  //parallel to X-Y
  if (slope.z() == 0) {
    if (IS_IN_CLOSE_RANGE(0.0, firstCorner.z(), lastCorner.z())) {
      //face x0
      double t = firstCorner.x() / slope.x();
      double y = t * slope.y();
      //double z = 0;
      int hitCount = 0;
      if (IS_IN_CLOSE_RANGE(y, firstCorner.y(), lastCorner.y())) {
        segBuffer.setStartPoint(m_firstCorner.x(), p0.y() + y, p0.z());
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
      if (IS_IN_CLOSE_RANGE(y, firstCorner.y(), lastCorner.y())) {
        SET_HIT_POINT(m_lastCorner.x(), p0.y() + y, p0.z());
      }

      //face y0
      t = firstCorner.y() / slope.y();
      double x = t * slope.x();
      if (IS_IN_CLOSE_RANGE(x, firstCorner.x(), lastCorner.x())) {
        SET_HIT_POINT(p0.x() + x, m_firstCorner.y(), p0.z());
      }

      //face y1
      t = lastCorner.y() / slope.y();
      x = t * slope.x();
      if (IS_IN_CLOSE_RANGE(x, firstCorner.x(), lastCorner.x())) {
        SET_HIT_POINT(p0.x() + x, m_lastCorner.y(), p0.z());
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
    if (IS_IN_CLOSE_RANGE(0.0, firstCorner.y(), lastCorner.y())) {
      //face x0
      double t = firstCorner.x() / slope.x();
      double z = t * slope.z();
      int hitCount = 0;
      if (IS_IN_CLOSE_RANGE(z, firstCorner.z(), lastCorner.z())) {
        SET_HIT_POINT(m_firstCorner.x(), p0.y(), p0.z() + z);
      }

      //face x1
      t = lastCorner.x() / slope.x();
      z = t * slope.z();
      if (IS_IN_CLOSE_RANGE(z, firstCorner.z(), lastCorner.z())) {
        SET_HIT_POINT(m_lastCorner.x(), p0.y(), p0.z() + z);
      }

      //face z0
      t = firstCorner.z() / slope.z();
      double x = t * slope.x();
      if (IS_IN_CLOSE_RANGE(x, firstCorner.x(), lastCorner.x())) {
        SET_HIT_POINT(p0.x() + x, p0.y(), m_firstCorner.z());
      }

      //face y1
      t = lastCorner.z() / slope.z();
      x = t * slope.x();
      if (IS_IN_CLOSE_RANGE(x, firstCorner.x(), lastCorner.x())) {
        SET_HIT_POINT(p0.x() + x, p0.y(), m_lastCorner.z());
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
    if (IS_IN_CLOSE_RANGE(0.0, firstCorner.x(), lastCorner.x())) {
      //face y0
      double t = firstCorner.y() / slope.y();
      double z = t * slope.z();
      int hitCount = 0;
      if (IS_IN_CLOSE_RANGE(z, firstCorner.z(), lastCorner.z())) {
        SET_HIT_POINT(p0.x(), m_firstCorner.y(), p0.z() + z);
      }

      //face y1
      t = lastCorner.y() / slope.y();
      z = t * slope.z();
      if (IS_IN_CLOSE_RANGE(z, firstCorner.z(), lastCorner.z())) {
        SET_HIT_POINT(p0.x(), m_lastCorner.y(), p0.z() + z);
      }

      //face z0
      t = firstCorner.z() / slope.z();
      double y = t * slope.y();
      if (IS_IN_CLOSE_RANGE(y, firstCorner.y(), lastCorner.y())) {
        SET_HIT_POINT(p0.x(), p0.y() + y, m_firstCorner.z());
      }

      //face z1
      t = lastCorner.z() / slope.z();
      y = t * slope.y();
      if (IS_IN_CLOSE_RANGE(y, firstCorner.y(), lastCorner.y())) {
        SET_HIT_POINT(p0.x(), p0.y() + y, m_lastCorner.z());
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
  if (IS_IN_CLOSE_RANGE(y, firstCorner.y(), lastCorner.y()) &&
      IS_IN_CLOSE_RANGE(z, firstCorner.z(), lastCorner.z())) {
    SET_HIT_POINT(m_firstCorner.x(), p0.y() + y, p0.z() + z);
  }

  //face x1
  t = lastCorner.x() / slope.x();
  y = t * slope.y();
  z = t * slope.z();
  if (IS_IN_CLOSE_RANGE(y, firstCorner.y(), lastCorner.y()) &&
      IS_IN_CLOSE_RANGE(z, firstCorner.z(), lastCorner.z())) {
    SET_HIT_POINT(m_lastCorner.x(), p0.y() + y, p0.z() + z);
  }

  //face y0
  t = firstCorner.y() / slope.y();
  double x = t * slope.x();
  z = t * slope.z();
  if (IS_IN_CLOSE_RANGE(x, firstCorner.x(), lastCorner.x()) &&
      IS_IN_CLOSE_RANGE(z, firstCorner.z(), lastCorner.z())) {
    SET_HIT_POINT(p0.x() + x, m_firstCorner.y(), p0.z() + z);
  }

  //face y1
  t = lastCorner.y() / slope.y();
  x = t * slope.x();
  z = t * slope.z();
  if (IS_IN_CLOSE_RANGE(x, firstCorner.x(), lastCorner.x()) &&
      IS_IN_CLOSE_RANGE(z, firstCorner.z(), lastCorner.z())) {
    SET_HIT_POINT(p0.x() + x, m_lastCorner.y(), p0.z() + z);
  }


  //face z0
  t = firstCorner.z() / slope.z();
  x = t * slope.x();
  y = t * slope.y();
  if (IS_IN_CLOSE_RANGE(x, firstCorner.x(), lastCorner.x()) &&
      IS_IN_CLOSE_RANGE(y, firstCorner.y(), lastCorner.y())) {
    SET_HIT_POINT(p0.x() + x, p0.y() + y, m_firstCorner.z());
  }

  //face z1
  t = lastCorner.z() / slope.z();
  x = t * slope.x();
  y = t * slope.y();
  if (IS_IN_CLOSE_RANGE(x, firstCorner.x(), lastCorner.x()) &&
      IS_IN_CLOSE_RANGE(y, firstCorner.y(), lastCorner.y())) {
    SET_HIT_POINT(p0.x() + x, p0.y() + y, m_lastCorner.z());
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
  cuboid.setFirstCorner(m_firstCorner.toIntPoint());
  cuboid.setLastCorner(m_lastCorner.toIntPoint());

  return cuboid;
}

double ZCuboid::getDiagonalLength() const
{
  return sqrt(width() * width() + height() * height() + depth() * depth());
}

std::vector<double> ZCuboid::toCornerVector() const
{
  std::vector<double> corner(6);
  corner[0] = firstCorner().x();
  corner[1] = lastCorner().x();

  corner[2] = firstCorner().y();
  corner[3] = lastCorner().y();

  corner[4] = firstCorner().z();
  corner[5] = lastCorner().z();

  return corner;
}
