#include "zmodelviewtransform.h"

#include "geometry/zgeometry.h"
#include "geometry/zrotator.h"
#include "zjsonobject.h"

ZModelViewTransform::ZModelViewTransform()
{

}

ZPoint ZModelViewTransform::transformWithoutOffset(const ZPoint &pt) const
{
  ZPoint newPt = pt;
  switch (m_sliceAxis) {
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
  case neutu::EAxis::Z:
    newPt.shiftSliceAxis(m_sliceAxis);
    break;
  case neutu::EAxis::ARB:
    newPt = m_cutPlane.getPlane().align(pt);
    break;
  }

  if (!m_rightHanded) {
    newPt.flipZ();
  }

  return newPt;
}

ZPoint ZModelViewTransform::transform(const ZPoint &pt) const
{
  ZPoint newPt = pt;
  switch (m_sliceAxis) {
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
  case neutu::EAxis::Z:
    newPt -= m_cutPlane.getOffset();
    newPt.shiftSliceAxis(m_sliceAxis);
    break;
  case neutu::EAxis::ARB:
    newPt = m_cutPlane.align(pt);
    break;
  }

  if (!m_rightHanded) {
    newPt.flipZ();
  }

  return newPt;
}

void ZModelViewTransform::transform(double *x, double *y, double *z) const
{
  ZPoint result = transform(ZPoint(*x, *y, *z));
  *x = result.getX();
  *y = result.getY();
  *z = result.getZ();
}

ZPoint ZModelViewTransform::inverseTransform(double u, double v, double n) const
{
  ZPoint pt(u, v, m_rightHanded ? n : -n);

  switch (m_sliceAxis) {
  case neutu::EAxis::ARB:
    pt = m_cutPlane.getV1() * u + m_cutPlane.getV2() * v +
        m_cutPlane.getNormal() * n + m_cutPlane.getOffset();
    break;
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
    pt.shiftSliceAxisInverse(m_sliceAxis);
  case neutu::EAxis::Z:
    pt += m_cutPlane.getOffset();
    break;
  }

  return pt;
}

ZPoint ZModelViewTransform::inverseTransform(const ZPoint &pt) const
{
  return inverseTransform(pt.getX(), pt.getY(), pt.getZ());
}


ZPoint ZModelViewTransform::transformBoxSize(const ZPoint &dim) const
{
  ZPoint pt = dim;
  switch (getSliceAxis()) {
  case neutu::EAxis::Y:
  case neutu::EAxis::X:
    pt.shiftSliceAxis(getSliceAxis());
    break;
  case neutu::EAxis::ARB:
  {
    double length = dim.length();
    pt.set(length, length, length);
  }
    break;
  default:
    break;
  }

  return pt;
}

ZPoint ZModelViewTransform::getCutPlaneNormal() const
{
  switch (m_sliceAxis) {
  case neutu::EAxis::X:
    return ZPoint(1, 0, 0);
  case neutu::EAxis::Y:
    return ZPoint(0, 1, 0);
  case neutu::EAxis::Z:
    return ZPoint(0, 0, 1);
  default:
    break;
  }

  return m_cutPlane.getNormal();
}

ZPoint ZModelViewTransform::getCutCenter() const
{
  return getCutPlane().getOffset();
}

/*
double ZModelViewTransform::getCutDepth() const
{
  double depth = 0.0;

  switch (m_sliceAxis) {
  case neutu::EAxis::X:
    depth = m_cutPlane.getOffset().getX();
    break;
  case neutu::EAxis::Y:
    depth = m_cutPlane.getOffset().getY();
    break;
  case neutu::EAxis::Z:
    depth = m_cutPlane.getOffset().getZ();
    break;
  case neutu::EAxis::ARB:
    break;
  }

  return depth;
}
*/

ZAffinePlane ZModelViewTransform::getCutPlane() const
{
  return m_cutPlane;
}

neutu::EAxis ZModelViewTransform::getSliceAxis() const
{
  return m_sliceAxis;
}

void ZModelViewTransform::setCutPlane(neutu::EAxis sliceAxis)
{
  m_sliceAxis = sliceAxis;
  switch (m_sliceAxis) {
  case neutu::EAxis::X:
    m_cutPlane.setPlane(ZPoint(0, 0, 1), ZPoint(0, 1, 0));
    break;
  case neutu::EAxis::Y:
    m_cutPlane.setPlane(ZPoint(1, 0, 0), ZPoint(0, 0, 1));
    break;
  case neutu::EAxis::Z:
    m_cutPlane.setPlane(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    break;
  case neutu::EAxis::ARB:
//    m_cutPlane.setPlane(
//          ZPoint(1, 1, 0).getNormalized(), ZPoint(1, -1, 0).getNormalized());
    break;
  }
}

void ZModelViewTransform::setCutPlane(neutu::EAxis sliceAxis, double cutDepth)
{
  setCutPlane(sliceAxis);
  m_cutPlane.addOffset(cutDepth, m_sliceAxis);
}

void ZModelViewTransform::setCutPlane(
    neutu::EAxis sliceAxis, const ZPoint &cutCenter)
{
  setCutPlane(sliceAxis);
  m_cutPlane.setOffset(cutCenter);
}

void ZModelViewTransform::moveCutDepth(double d)
{
  m_cutPlane.addOffset(m_rightHanded ? d : -d, getSliceAxis());
}

double ZModelViewTransform::getCutDepth(const ZPoint &startPlane) const
{
  double d = 0.0;
  if (getSliceAxis() == neutu::EAxis::ARB) {
    d = (getCutCenter() - startPlane).dot(getCutPlaneNormal());
  } else {
    d = getCutCenter().getValue(getSliceAxis()) -
        startPlane.getValue(getSliceAxis());
  }

  return m_rightHanded ? d : -d;
}

void ZModelViewTransform::setCutDepth(const ZPoint &startPlane, double d)
{
  double d0 = 0;

  if (getSliceAxis() == neutu::EAxis::ARB) {
    d0 = (startPlane - getCutCenter()).dot(getCutPlaneNormal());
  } else {
    d0 = (getCutCenter() - startPlane).getValue(getSliceAxis());
  }

  if (!m_rightHanded) {
    d0 = -d0;
  }

  moveCutDepth(d - d0);
}

void ZModelViewTransform::setCutPlane(const ZAffinePlane &plane)
{
  m_cutPlane = plane;
  m_sliceAxis = neutu::EAxis::ARB;
}

void ZModelViewTransform::setCutPlane(
    const ZPoint &center, const ZPoint &v1, const ZPoint &v2)
{
  setCutPlane(ZAffinePlane(center, v1, v2));
}

void ZModelViewTransform::setCutPlane(const ZPoint &v1, const ZPoint &v2)
{
  m_cutPlane.setPlane(v1, v2);
  m_sliceAxis = neutu::EAxis::ARB;
}

void ZModelViewTransform::setCutCenter(const ZPoint &pt)
{
  setCutCenter(pt.getX(), pt.getY(), pt.getZ());
}

void ZModelViewTransform::setCutCenter(double x, double y, double z)
{
  m_cutPlane.setOffset(x, y, z);
  /*
  switch (m_sliceAxis) {
  case neutu::EAxis::ARB:
    m_cutPlane.setOffset(ZPoint(x, y, z));
    break;
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
  case neutu::EAxis::Z:
  {
    double xShift = x;
    double yShift = y;
    double zShift = z;
    zgeom::shiftSliceAxis(xShift, yShift, zShift, m_sliceAxis);
    if (xShift != 0.0 || yShift != 0.0) {
      m_sliceAxis = neutu::EAxis::ARB;
      setCutCenter(x, y, z);
    } else {
      setCutDepth(zShift);
    }
  }
    break;
  }
  */
}

void ZModelViewTransform::setRightHanded(bool r)
{
  m_rightHanded = r;
}

bool ZModelViewTransform::rightHanded() const
{
  return m_rightHanded;
}

ZJsonObject ZModelViewTransform::toJsonObject() const
{
  ZJsonObject obj;

  switch (m_sliceAxis) {
  case neutu::EAxis::X:
    obj.setEntry("slice", "X");
    break;
  case neutu::EAxis::Y:
    obj.setEntry("slice", "Y");
    break;
  case neutu::EAxis::Z:
    obj.setEntry("slice", "Z");
    break;
  case neutu::EAxis::ARB:
  {
    ZJsonObject sliceObj;
    sliceObj.setEntry(
          "offset", m_cutPlane.getOffset().toArray());
    sliceObj.setEntry("v1", m_cutPlane.getV1().toArray());
    sliceObj.setEntry("v2", m_cutPlane.getV2().toArray());
    obj.setEntry("slice", sliceObj);
  }
    break;
  }

  if (!m_rightHanded) {
    obj.setEntry("hand", "left");
  }

  return obj;
}

bool ZModelViewTransform::operator==(const ZModelViewTransform &t) const
{
  return m_sliceAxis == t.m_sliceAxis && m_cutPlane == t.m_cutPlane;
}

void ZModelViewTransform::translateCutCenterOnPlane(double du, double dv)
{
  m_cutPlane.translateOnPlane(du, dv);
}

bool ZModelViewTransform::onSamePlane(const ZModelViewTransform &t) const
{
  return m_cutPlane.onSamePlane(t.getCutPlane());
}

bool ZModelViewTransform::hasSamePlane(const ZModelViewTransform &t) const
{
  return m_cutPlane.hasSamePlane(t.getCutPlane());
}

void ZModelViewTransform::rotate(double au, double av, double rad)
{
  if (std::fabs(au) <= ZPoint::MIN_DIST && std::fabs(av) <= ZPoint::MIN_DIST) {
    au = 1.0;
    av = 0.0;
  }

  ZPoint axis = m_cutPlane.getV1() * au + m_cutPlane.getV2() * av;
  ZRotator rotator(axis, rad);
  m_cutPlane.setPlane(rotator.rotate(m_cutPlane.getPlane()));
}

std::ostream& operator<< (
    std::ostream &stream, const ZModelViewTransform &t)
{
   stream << t.m_cutPlane;

   return stream;
}

void ZModelViewTransform::copyWithoutOrientation(const ZModelViewTransform &t)
{
  m_cutPlane.setOffset(t.m_cutPlane.getOffset());
}
