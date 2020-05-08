#include "zworldviewtransform.h"

ZWorldViewTransform::ZWorldViewTransform()
{

}

ZPoint ZWorldViewTransform::transform(const ZPoint &pt) const
{
  ZPoint newPt = pt;
  switch (m_sliceAxis) {
  case neutu::EAxis::X:
  case neutu::EAxis::Y:
    newPt.shiftSliceAxis(m_sliceAxis);
    break;
  case neutu::EAxis::ARB:
    newPt = m_cutPlane.align(pt);
    break;
  default:
    break;
  }

  return newPt;
}

double ZWorldViewTransform::getCutDepth() const
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

ZAffinePlane ZWorldViewTransform::getCutPlane() const
{
  return m_cutPlane;
}

neutu::EAxis ZWorldViewTransform::getSliceAxis() const
{
  return m_sliceAxis;
}

void ZWorldViewTransform::setCutPlane(neutu::EAxis sliceAxis, double cutDepth)
{
  m_sliceAxis = sliceAxis;
  switch (m_sliceAxis) {
  case neutu::EAxis::X:
    m_cutPlane.set(ZPoint(cutDepth, 0, 0), ZPoint(0, 0, 1), ZPoint(0, 1, 0));
    break;
  case neutu::EAxis::Y:
    m_cutPlane.set(ZPoint(0, cutDepth, 0), ZPoint(1, 0, 0), ZPoint(0, 0, 1));
    break;
  case neutu::EAxis::Z:
    m_cutPlane.set(ZPoint(0, 0, cutDepth), ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    break;
  case neutu::EAxis::ARB:
    m_cutPlane.translateDepth(cutDepth);
    break;
  }
}

void ZWorldViewTransform::setCutPlane(const ZAffinePlane &plane)
{
  m_cutPlane = plane;
  m_sliceAxis = neutu::EAxis::ARB;
}
