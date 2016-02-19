#include "zintcuboidface.h"
#include <iostream>
#include <cmath>

#include "tz_utilities.h"
#include "zintpoint.h"
#include "zswcgenerator.h"
#include "zswctree.h"

ZIntCuboidFace::ZIntCuboidFace() : m_z(0), m_normalAxis(NeuTube::Z_AXIS),
  m_isNormalPositive(true)
{
}

bool ZIntCuboidFace::isValid() const
{
  return (getLowerBound(0) <= getUpperBound(0)) &&
      (getLowerBound(1) <= getUpperBound(1));
}

int ZIntCuboidFace::getLowerBound(int index) const
{
  if (index == 0) {
    return getFirstCorner().getX();
  } else {
    return getFirstCorner().getY();
  }
}

int ZIntCuboidFace::getUpperBound(int index) const
{
  if (index == 0) {
    return getLastCorner().getX();
  } else {
    return getLastCorner().getY();
  }
}

int ZIntCuboidFace::getPlanePosition() const
{
  return m_z;
}

ZIntCuboidFace::Corner ZIntCuboidFace::getCorner(int index) const
{
  switch (index) {
  case 0:
    return getFirstCorner();
  case 1:
    return Corner(getUpperBound(0), getLowerBound(1));
  case 2:
    return Corner(getLowerBound(0), getUpperBound(1));
  case 3:
    return getLastCorner();
  }

  return Corner(0, 0);
}

bool ZIntCuboidFace::hasOverlap(const ZIntCuboidFace &face) const
{
  if (getAxis() != face.getAxis() ||
      getPlanePosition() != face.getPlanePosition()) {
    return false;
  }

  if (getLowerBound(0) > face.getUpperBound(0)) {
    return false;
  }

  if (getLowerBound(1) > face.getUpperBound(1)) {
    return false;
  }

  if (face.getLowerBound(0) > getUpperBound(0)) {
    return false;
  }

  if (face.getLowerBound(1) > getUpperBound(1)) {
    return false;
  }

  return true;
}

bool ZIntCuboidFace::isWithin(const ZIntCuboidFace &face) const
{
  return face.contains(getFirstCorner()) && face.contains(getLastCorner());
}

void ZIntCuboidFace::print() const
{
  std::cout << "Normal axis: ";
  if (m_isNormalPositive) {
    std::cout << "+";
  } else {
    std::cout << "-";
  }

  switch (getAxis()) {
  case NeuTube::X_AXIS:
    std::cout << "X";
    break;
  case NeuTube::Y_AXIS:
    std::cout << "Y";
    break;
  case NeuTube::Z_AXIS:
    std::cout << "Z";
    break;
  default:
    break;
  }
  std::cout << "(" << m_z << ")";
  std::cout << "; ";
  std::cout << "(" << getFirstCorner().getX() << ", " << getFirstCorner().getY()
            << ") --> (" << getLastCorner().getX() << ", "
            << getLastCorner().getY() << ")" << std::endl;
}

bool ZIntCuboidFace::contains(ZIntCuboidFace::Corner pt) const
{
  return (pt.getX() >= getLowerBound(0) && pt.getX() <= getUpperBound(0) &&
          pt.getY() >= getLowerBound(1) && pt.getY() <= getUpperBound(1));
}

bool ZIntCuboidFace::contains(int x, int y, int z) const
{
  int u = 0;
  int v = 0;
  int w = 0;

  switch (getAxis()) {
  case NeuTube::X_AXIS:
    u = y;
    v = z;
    w = x;
    break;
  case NeuTube::Y_AXIS:
    u = x;
    v = z;
    w = y;
    break;
  case NeuTube::Z_AXIS:
    u = x;
    v = y;
    w = z;
    break;
  }

  if (w != getPlanePosition()) {
    return false;
  }

  return getLowerBound(0) <= u && getUpperBound(0) >= u &&
      getLowerBound(1) <= v && getUpperBound(1) >= v;
}

void ZIntCuboidFace::set(
    const Corner &firstCorner, const Corner &lastCorner)
{
  m_firstCorner = firstCorner;
  m_lastCorner = lastCorner;
}

void ZIntCuboidFace::set(int x0, int y0, int x1, int y1)
{
  m_firstCorner.set(x0, y0);
  m_lastCorner.set(x1, y1);
}

void ZIntCuboidFace::setNormal(NeuTube::EAxis axis)
{
  m_normalAxis = axis;
}

void ZIntCuboidFace::setNormal(NeuTube::EAxis axis, bool isPositive)
{
  m_normalAxis = axis;
  m_isNormalPositive = isPositive;
}

void ZIntCuboidFace::setZ(int z)
{
  m_z = z;
}

void ZIntCuboidFace::flip()
{
  m_isNormalPositive = !m_isNormalPositive;
}

void ZIntCuboidFace::moveAxis(int dz)
{
  m_z += dz;
}

void ZIntCuboidFace::moveBackward(int dz)
{
  if (isNormalPositive()) {
    m_z -= dz;
  } else {
    m_z += dz;
  }
}

void ZIntCuboidFace::moveForward(int dz)
{
  if (isNormalPositive()) {
    m_z += dz;
  } else {
    m_z -= dz;
  }
}

ZIntCuboidFaceArray ZIntCuboidFace::cropBy(
    const ZIntCuboidFaceArray &faceArray) const
{
  ZIntCuboidFaceArray result;
  result.appendValid(*this);

  for (ZIntCuboidFaceArray::const_iterator iter = faceArray.begin();
       iter != faceArray.end(); ++iter) {
    const ZIntCuboidFace &face = *iter;
    result = result.cropBy(face);
    if (result.empty()) { //No need to continue on empty face
      break;
    }
  }

  return result;
}

ZIntPoint ZIntCuboidFace::getCornerCoordinates(int index) const
{
  ZIntPoint pt;
  Corner corner = getCorner(index);
  switch (getAxis()) {
  case NeuTube::X_AXIS:
    pt.set(getPlanePosition(), corner.getX(), corner.getY());
    break;
  case NeuTube::Y_AXIS:
    pt.set(corner.getX(), getPlanePosition(), corner.getY());
    break;
  case NeuTube::Z_AXIS:
    pt.set(corner.getX(), corner.getY(), getPlanePosition());
    break;
  }

  return pt;
}

ZIntCuboidFaceArray ZIntCuboidFace::cropBy(const ZIntCuboidFace &face) const
{
  ZIntCuboidFaceArray faceArray;
  if (hasOverlap(face)) {
    if (isWithin(face)) {
      return faceArray;
    } else {
      ZIntCuboidFace subface(getAxis(), isNormalPositive());
      subface.setZ(getPlanePosition());

      subface.set(getFirstCorner(),
                  Corner(getUpperBound(0), face.getLowerBound(1) - 1));
      faceArray.appendValid(subface);

      subface.set(Corner(getLowerBound(0),
                         imax2(getLowerBound(1), face.getLowerBound(1))),
                  Corner(face.getLowerBound(0) - 1, getUpperBound(1)));
      faceArray.appendValid(subface);

      subface.set(Corner(face.getUpperBound(0) + 1,
                         imax2(getLowerBound(1), face.getLowerBound(1))),
                  getLastCorner());
      faceArray.appendValid(subface);

      subface.set(Corner(imax2(getLowerBound(0), face.getLowerBound(0)),
                         face.getUpperBound(1) + 1),
                  Corner(imin2(getUpperBound(0), face.getUpperBound(0)),
                         getUpperBound(1)));
      faceArray.appendValid(subface);
    }
#if 0
    else if (face.isWithin(*this)) {
      ZIntCuboidFace subface(getAxis(), isNormalPositive());
      secondCorner.set(getUpperBound(0), face.getLowerBound(1));
      subface.set(getFirstCorner(), secondCorner);
      faceArray.appendValid(subface);

      subface.set(Corner(getLowerBound(0), face.getLowerBound(1)),
                  face.getCorner(2));
      faceArray.appendValid(subface);

      subface.set(Corner(getLowerBound(0), face.getUpperBound(1)),
                  getLastCorner());
      faceArray.appendValid(subface);
    } else {

    }
#endif
  } else {
    faceArray.appendValid(*this);
  }

  return faceArray;
}

double ZIntCuboidFace::computeDistance(double x, double y, double z)
{
  double du = 0.0;
  double dv = 0.0;
  double dw = 0.0;

  double u = 0.0;
  double v = 0.0;
  double w = 0.0;

  switch (getAxis()) {
  case NeuTube::X_AXIS:
    u = y;
    v = z;
    w = x;
    break;
  case NeuTube::Y_AXIS:
    u = x;
    v = z;
    w = y;
    break;
  case NeuTube::Z_AXIS:
    u = x;
    v = y;
    w = z;
    break;
  }

  if ((double) getLowerBound(0) > u) {
    du = getLowerBound(0) - u;
  } else if ((double) getUpperBound(0) < u) {
    du = u - getUpperBound(0);
  }

  if ((double) getLowerBound(1) > v) {
    dv = getLowerBound(1) - v;
  } else if ((double) getUpperBound(1) < v) {
    dv = v - getUpperBound(1);
  }

  dw = getPlanePosition() - w;

  return sqrt(du * du + dv * dv + dw * dw);
}

void ZIntCuboidFaceArray::appendValid(const ZIntCuboidFace &face)
{
  if (face.isValid()) {
    push_back(face);
  }
}

void ZIntCuboidFaceArray::append(const ZIntCuboidFace &face)
{
  push_back(face);
}

void ZIntCuboidFaceArray::append(const ZIntCuboidFaceArray &faceArray)
{
  insert(end(), faceArray.begin(), faceArray.end());
}

void ZIntCuboidFaceArray::print() const
{
  std::cout << size() << " face(s)" << std::endl;

  for (const_iterator iter = begin(); iter != end(); ++iter) {
    const ZIntCuboidFace &face = *iter;
    face.print();
  }
}

void ZIntCuboidFaceArray::exportSwc(const std::string &filePath) const
{
  if (!empty()) {
    ZSwcTree *tree = new ZSwcTree;
    int index = 0;
    for (const_iterator iter = begin(); iter != end();
         ++iter, ++index) {
      const ZIntCuboidFace &face = *iter;
      ZCuboid cuboid;
      cuboid.set(face.getCornerCoordinates(0), face.getCornerCoordinates(3));
      ZSwcTree *subtree = ZSwcTree::CreateCuboidSwc(cuboid);
      subtree->setType(index);
      tree->merge(subtree, true);
    }

    tree->resortId();
    tree->save(filePath);

    delete tree;
  }
}

void ZIntCuboidFaceArray::append(const Cuboid_I *cuboid)
{
  if (cuboid != NULL) {
    if (Cuboid_I_Is_Valid(cuboid)) {
      ZIntCuboidFace face;

      face.setNormal(NeuTube::X_AXIS, true);
      face.setZ(cuboid->cb[0]);
      face.set(cuboid->cb[1], cuboid->cb[2], cuboid->ce[1], cuboid->ce[2]);
      append(face);

      //face.setNormal(NeuTube::X_AXIS, false);
      face.flip();
      face.setZ(cuboid->ce[0]);
      append(face);

      face.setNormal(NeuTube::Y_AXIS, true);
      face.setZ(cuboid->cb[1]);
      face.set(cuboid->cb[0], cuboid->cb[2], cuboid->ce[0], cuboid->ce[2]);
      append(face);

      face.flip();
      face.setZ(cuboid->ce[1]);
      append(face);

      face.setNormal(NeuTube::Z_AXIS, true);
      face.setZ(cuboid->cb[2]);
      face.set(cuboid->cb[0], cuboid->cb[1], cuboid->ce[0], cuboid->ce[1]);
      append(face);

      face.flip();
      //face.setNormal(NeuTube::Z_AXIS, false);
      face.setZ(cuboid->ce[2]);
      append(face);
    }
  }
}

ZIntCuboidFaceArray ZIntCuboidFaceArray::cropBy(const ZIntCuboidFace &face)
{
  ZIntCuboidFaceArray faceArray;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    const ZIntCuboidFace &sourceFace = *iter;
    faceArray.append(sourceFace.cropBy(face));
  }

  return faceArray;
}

ZIntCuboidFaceArray ZIntCuboidFaceArray::cropBy(
    const ZIntCuboidFaceArray &faceArray)
{
  ZIntCuboidFaceArray result;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    const ZIntCuboidFace &sourceFace = *iter;
    result.append(sourceFace.cropBy(faceArray));
  }

  return result;
}

void ZIntCuboidFaceArray::moveBackward(int dz)
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    ZIntCuboidFace &sourceFace = *iter;
    sourceFace.moveBackward(dz);
  }
}

bool ZIntCuboidFaceArray::contains(int x, int y, int z) const
{
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    const ZIntCuboidFace &face = *iter;
    if (face.contains(x, y, z)) {
      return true;
    }
  }

  return false;
}
