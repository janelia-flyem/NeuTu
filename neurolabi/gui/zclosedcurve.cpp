#include "zclosedcurve.h"
#include <iostream>
#include <algorithm>

#include "tz_cdefs.h"
#include "tz_utilities.h"
#include "zjsonobject.h"
#include "neutubeconfig.h"
#include "zjsonarray.h"
#include "zjsonparser.h"
#include "zswctree.h"
#include "swctreenode.h"
#include "zlinesegment.h"

const ZPoint ZClosedCurve::m_emptyPlaceHolder;

ZClosedCurve::ZClosedCurve()
{
}

bool ZClosedCurve::isEmpty() const
{
  return m_landmarkArray.empty();
}

size_t ZClosedCurve::getLandmarkNumber() const
{
  return m_landmarkArray.size();
}

const ZPoint& ZClosedCurve::getLandmark(int index) const
{
  if (isEmpty()) {
    return m_emptyPlaceHolder;
  }

  if (index < 0) {
    index += ((-index - 1) / ((int) m_landmarkArray.size()) + 1) *
        (int) m_landmarkArray.size();
  }

  index %= (int) m_landmarkArray.size();
  return m_landmarkArray[index];
}

int ZClosedCurve::findMatchShift(const ZClosedCurve &target) const
{
  ZPoint center1 = computeCenter();
  ZPoint center2 = target.computeCenter();

  int shift = 0;
  double maxCosAngle = 0.0;

  for (size_t i = 0; i < this->getLandmarkNumber(); ++i) {
    for (size_t j = 0; j < target.getLandmarkNumber(); ++j) {
      ZPoint pt1 = getLandmark(i) - center1;
      ZPoint pt2 = target.getLandmark(j) - center2;
      double cosAngle = pt1.cosAngle(pt2);
      if (cosAngle > maxCosAngle) {
        shift = i - j;
        maxCosAngle = cosAngle;
      }
    }
  }

#if 0
  int index1 = (int) getMinXIndex();
  int index2 = (int) target.getMinXIndex();

  int shift = index1 - index2;
  ZPoint pt1 = getLandmark(index1) - center1;
  ZPoint pt2 = target.getLandmark(index2) - center2;

  pt1.setZ(0);
  pt1.normalize();
  pt2.setZ(0);
  pt2.normalize();

  double diff = pt1.distanceTo(pt2);

  index1 = (int) getMinYIndex();
  index2 = (int) target.getMinYIndex();
  pt1 = getLandmark(index1) - center1;
  pt2 = target.getLandmark(index2) - center2;

  pt1.setZ(0);
  pt1.normalize();
  pt2.setZ(0);
  pt2.normalize();

  if (diff > pt1.distanceTo(pt2)) {
    shift = index1 - index2;
  }
#endif

  return shift;
}

size_t ZClosedCurve::getMinXIndex() const
{
  size_t index = 0;

  double minX = Infinity;
  for (size_t i = 0; i < m_landmarkArray.size(); ++i) {
    const ZPoint &pt = m_landmarkArray[i];
    if (minX > pt.x()) {
      minX = pt.x();
      index = i;
    }
  }

  return index;
}

size_t ZClosedCurve::getMinYIndex() const
{
  size_t index = 0;

  double minY = Infinity;
  for (size_t i = 0; i < m_landmarkArray.size(); ++i) {
    const ZPoint &pt = m_landmarkArray[i];
    if (minY > pt.y()) {
      minY = pt.y();
      index = i;
    }
  }

  return index;
}

double ZClosedCurve::getLength() const
{
  double length = 0.0;
  if (getSegmentNumber() > 0) {
    for (size_t i = 1; i < m_landmarkArray.size(); ++i) {
      const ZPoint &start = m_landmarkArray[i - 1];
      const ZPoint &end = m_landmarkArray[i];
      length += start.distanceTo(end);
    }
    length += m_landmarkArray.back().distanceTo(m_landmarkArray.front());
  }

  return length;
}

size_t ZClosedCurve::getSegmentNumber() const
{
  if (m_landmarkArray.size() <= 1) {
    return 0;
  }

  return m_landmarkArray.size();
}

ZLineSegment ZClosedCurve::getSegment(size_t index) const
{
  ZLineSegment seg;
  if (!isEmpty()) {
    if (index >= getSegmentNumber()) {
      index %= getSegmentNumber();
    }

    if (index < getSegmentNumber()) {
      seg.setStartPoint(m_landmarkArray[index]);
      if (index == m_landmarkArray.size() - 1) {
        seg.setEndPoint(m_landmarkArray[0]);
      } else {
        seg.setEndPoint(m_landmarkArray[index + 1]);
      }
    }
  }

  return seg;
}

ZClosedCurve ZClosedCurve::resampleF(int n) const
{
  ZClosedCurve curve;
  if (n <= 0 || isEmpty()) {
    return curve;
  }

  curve.append(getLandmark(0));

  if (n > 1) {
    double length = getLength();
    double step = length / n;

    double remainedLength = step;
    ZLineSegment currentSeg = getSegment(0);
    double currentSegLength = currentSeg.getLength();

    size_t currentSegIndex = 0;
    bool endReached = false;
    for (int i = 1; i < n; ++i) {
      while (currentSegLength < remainedLength) {
        remainedLength -= currentSegLength;
        ++currentSegIndex;
        if (currentSegIndex < getSegmentNumber()) {
          currentSeg = getSegment(currentSegIndex);
          currentSegLength = currentSeg.getLength();
        } else {
          endReached = true;
          break;
        }
      }
      if (endReached) {
        break;
      }
      curve.append(currentSeg.getInterpolation(remainedLength));
      remainedLength += step;
    }
  }

  return curve;
}

void ZClosedCurve::append(const ZPoint &pt)
{
  m_landmarkArray.push_back(pt);
}

void ZClosedCurve::append(double x, double y, double z)
{
  append(ZPoint(x, y, z));
}

ZClosedCurve* ZClosedCurve::interpolate(
    const ZClosedCurve &curve, double lambda, int shift, ZClosedCurve *result)
{
  if (result != NULL) {
    result->clear();
  }

  if (!isEmpty() && !curve.isEmpty()) {
    if (result == NULL) {
      result = new ZClosedCurve;
    }
    for (size_t i = 0; i < m_landmarkArray.size(); ++i) {
      ZPoint pt1 = getLandmark(i + shift);
      ZPoint pt2 = curve.getLandmark(i);
      ZPoint pt = pt1 * lambda + pt2 * (1 - lambda);

      result->append(pt);
    }
  }

  return result;
}

ZClosedCurve ZClosedCurve::interpolate(
    const ZClosedCurve &curve, double lambda, int shift)
{
#ifdef _DEBUG_2
  ZSwcTree tree;
  tree.forceVirtualRoot();
  for (size_t i = 0; i < m_landmarkArray.size(); ++i) {
    ZPoint pt1 = getLandmark(i + shift);
    ZPoint pt2 = curve.getLandmark(i);
    Swc_Tree_Node *tn = SwcTreeNode::makePointer(pt1, 5.0);
    Swc_Tree_Node *tn2 = SwcTreeNode::makePointer(pt2, 5.0);
    SwcTreeNode::setParent(tn2, tn);
    SwcTreeNode::setParent(tn, tree.root(), SwcTreeNode::CHILD_POS_FIRST);
  }
  tree.save(GET_DATA_DIR + "/test.swc");
#endif

  ZClosedCurve result;
  interpolate(curve, lambda, shift, &result);

  return result;
}

void ZClosedCurve::print() const
{
  std::cout << "Closed curve:" << std::endl;
  for (size_t i = 0; i < m_landmarkArray.size(); ++i) {
    const ZPoint &pt = m_landmarkArray[i];
    std::cout << "  " << pt.toString() << std::endl;
  }
}

void ZClosedCurve::loadJsonObject(const ZJsonObject &obj)
{
  clear();

  if (obj.hasKey("curve")) {
    ZJsonArray array(obj["curve"], ZJsonValue::SET_INCREASE_REF_COUNT);
    for (size_t i = 0; i < array.size(); ++i) {
      ZJsonArray ptObj(array.at(i), ZJsonValue::SET_INCREASE_REF_COUNT);
      std::vector<double> coords = ptObj.toNumberArray();
      if (coords.size() == 3) {
        append(coords[0], coords[1], coords[2]);
      }
    }
  }
}

void ZClosedCurve::clear()
{
  m_landmarkArray.clear();
}

ZJsonObject ZClosedCurve::toJsonObject() const
{
  ZJsonObject obj;
  ZJsonArray curveObj(json_array(), ZJsonValue::SET_AS_IT_IS);

  for (size_t i = 0; i < m_landmarkArray.size(); ++i) {
    const ZPoint &pt = m_landmarkArray[i];
    ZJsonArray ptObj;
    ptObj.append(pt.x());
    ptObj.append(pt.y());
    ptObj.append(pt.z());

    curveObj.append(ptObj);
  }

  obj.setEntry("curve", curveObj);

  return obj;
}

void ZClosedCurve::reverse()
{
  std::reverse(m_landmarkArray.begin(), m_landmarkArray.end());
}

ZPoint ZClosedCurve::computeCenter() const
{
  return m_landmarkArray.computeCenter();
}

ZPoint ZClosedCurve::computeDirection() const
{
  ZPoint d;
  int nseg = getSegmentNumber();
  if (nseg > 0) {
    ZPoint center= computeCenter();

    for (int i = 0; i < nseg; ++i) {
      ZLineSegment seg = getSegment(i);
      d += (seg.getEndPoint() - center).cross(seg.getStartPoint() - center);
    }
    d /= nseg;
  }

  return d;
}

ZCuboid ZClosedCurve::getBoundBox() const
{
  return m_landmarkArray.getBoundBox();
}

void ZClosedCurve::scale(double sx, double sy, double sz)
{
  for (ZPointArray::iterator iter = m_landmarkArray.begin();
       iter != m_landmarkArray.end(); ++iter) {
    ZPoint &pt = *iter;
    pt *= ZPoint(sx, sy, sz);
  }
}

void ZClosedCurve::translate(double dx, double dy)
{
  for (ZPointArray::iterator iter = m_landmarkArray.begin();
       iter != m_landmarkArray.end(); ++iter) {
    ZPoint &pt = *iter;
    pt += ZPoint(dx, dy, 0);
  }
}

ZClosedCurve* ZClosedCurve::clone() const
{
  ZClosedCurve *curve = new ZClosedCurve(*this);

  return curve;
}
