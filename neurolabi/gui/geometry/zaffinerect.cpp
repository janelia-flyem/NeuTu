#include "zaffinerect.h"

ZAffineRect::ZAffineRect()
{

}

void ZAffineRect::set(
    const ZPoint &offset, const ZPoint &v1, const ZPoint &v2,
    double width, double height)
{
  m_ap.setOffset(offset);
  m_ap.setPlane(v1, v2);
  m_width = width;
  m_height = height;
}

double ZAffineRect::getWidth() const
{
  return m_width;
}

double ZAffineRect::getHeight() const
{
  return m_height;
}

ZPoint ZAffineRect::getV1() const
{
  return m_ap.getV1();
}

ZPoint ZAffineRect::getV2() const
{
  return m_ap.getV2();
}

ZPoint ZAffineRect::getCenter() const
{
  return m_ap.getOffset();
}

void ZAffineRect::setCenter(const ZPoint &offset)
{
  m_ap.setOffset(offset);
}

void ZAffineRect::setCenter(double x, double y, double z)
{
  setCenter(ZPoint(x, y, z));
}

void ZAffineRect::setPlane(const ZPoint &v1, const ZPoint &v2)
{
  m_ap.setPlane(v1, v2);
}

void ZAffineRect::setPlane(const ZAffinePlane &plane)
{
  m_ap = plane;
}

void ZAffineRect::setSize(double width, double height)
{
  m_width = width;
  m_height = height;
}

void ZAffineRect::translate(double dx, double dy, double dz)
{
  m_ap.translate(dx, dy, dz);
}

void ZAffineRect::translate(const ZPoint &dv)
{
  m_ap.translate(dv);
}

void ZAffineRect::scale(double su, double sv)
{
  m_width *= su;
  m_height *= sv;
}

ZAffinePlane ZAffineRect::getAffinePlane() const
{
  return m_ap;
}

bool ZAffineRect::containsProjection(const ZPoint &pt) const
{
  ZPoint dp = pt - getCenter();

  double u = dp.dot(getV1());
  double halfWidth = m_width / 2.0;
  if (u < -halfWidth || u > halfWidth) {
    return false;
  }

  double v = dp.dot(getV2());
  double halfHeight = m_height / 2.0;
  if (v < -halfHeight || v > halfHeight) {
    return false;
  }

  return true;
}

bool ZAffineRect::contains(const ZPoint &pt) const
{
  if (m_ap.contains(pt)) {
    return containsProjection(pt);
  }

  return false;
}

bool ZAffineRect::contains(const ZAffineRect &rect) const
{
  for (int i = 0; i < 4; ++i) {
    if (!contains(rect.getCorner(i))) {
      return false;
    }
  }

  return true;
}

bool ZAffineRect::contains(const ZPoint &pt, double d) const
{
  double dist = m_ap.getPlane().computeSignedDistance(pt);
  if (dist >= -d && dist < d) {
    return containsProjection(pt);
  }

  return false;
}

bool ZAffineRect::contains(const ZAffineRect &rect, double d) const
{
  if (m_ap == rect.m_ap) {
    return (m_width <= rect.m_width) && (m_height <= rect.m_height);
  }

  bool contained = false;
  if (m_ap.contains(rect.m_ap, d)) {
    contained = true;
    for (int i = 0; i < 4; ++i) {
      if (!containsProjection(rect.getCorner(i))) {
        contained = false;
        break;
      }
    }
  }

  return contained;
}

ZPoint ZAffineRect::getCorner(int index) const
{
  ZPoint pt;
  pt.invalidate();

  ZPoint xSpan = m_ap.getV1() * m_width / 2.0;
  ZPoint ySpan = m_ap.getV2() * m_height / 2.0;

  switch (index) {
  case 0:
    pt = m_ap.getOffset() + xSpan + ySpan;
    break;
  case 1:
    pt = m_ap.getOffset() - xSpan + ySpan;
    break;
  case 2:
    pt = m_ap.getOffset() - xSpan - ySpan;
    break;
  case 3:
    pt = m_ap.getOffset() + xSpan - ySpan;
    break;
  }

  return pt;
}

ZLineSegment ZAffineRect::getSide(int index) const
{
  if (index >= 0) {
    int c1 = index;
    int c2 = index + 1;
    if (index <= 3) {
      if (index == 3) {
        c2 = 0;
      }
      return ZLineSegment(getCorner(c1), getCorner(c2));
    }
  }


  return ZLineSegment(ZPoint::INVALID_POINT, ZPoint::INVALID_POINT);
}

bool ZAffineRect::operator== (const ZAffineRect &p) const
{
  return (m_width == p.m_width) && (m_height == p.m_height) && (m_ap == p.m_ap);
}

bool ZAffineRect::operator!= (const ZAffineRect &p) const
{
  return (m_width != p.m_width) || (m_height != p.m_height) || (m_ap != p.m_ap);
}

std::ostream& operator<<(std::ostream& stream, const ZAffineRect &r)
{
  stream << r.getAffinePlane()
         << " [" << r.getWidth() << "x" << r.getHeight() << "]";

  return stream;
}

/////////////////////////////////////

ZAffineRectBuilder::ZAffineRectBuilder()
{
}

ZAffineRectBuilder::ZAffineRectBuilder(int width, int height)
{
  m_ar.setSize(width, height);
}

ZAffineRectBuilder& ZAffineRectBuilder::at(const ZPoint &center)
{
  m_ar.setCenter(center);

  return *this;
}

ZAffineRectBuilder&ZAffineRectBuilder::on(const ZPoint &v1, const ZPoint &v2)
{
  m_ar.setPlane(v1, v2);

  return *this;
}

ZAffineRectBuilder&ZAffineRectBuilder::withSize(int width, int height)
{
  m_ar.setSize(width, height);

  return *this;
}

ZAffineRectBuilder::operator ZAffineRect() const
{
  return m_ar;
}
