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
  setSize(width, height);
}

double ZAffineRect::getWidth() const
{
  return m_width;
}

double ZAffineRect::getHeight() const
{
  return m_height;
}

bool ZAffineRect::isEmpty() const
{
  return m_width <= 0.0 || m_height <= 0.0;
}

bool ZAffineRect::isNonEmpty() const
{
  return !isEmpty();
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

void ZAffineRect::setPlane(const ZPlane &plane)
{
  m_ap.setPlane(plane);
}

void ZAffineRect::setPlane(const ZAffinePlane &plane)
{
  m_ap = plane;
}

void ZAffineRect::setSize(double width, double height)
{
  m_width = std::max(0.0, width);
  m_height = std::max(0.0, height);
}

void ZAffineRect::addSize(double dw, double dh)
{
  setSize(getWidth() + dw, getHeight() + dh);
}

void ZAffineRect::setSizeWithCornerFixed(
    double width, double height, int cornerIndex)
{
  if (cornerIndex < 0 || cornerIndex > 3) {
    setSize(width, height);
  } else {
    if (width < 0.0) {
      width = 0.0;
    }

    if (height < 0.0) {
      height = 0.0;
    }

    if (width != m_width || height != m_height) {
      double du = m_width - width;
      double dv = m_height - height;
      setSize(width, height);
      switch (cornerIndex) {
      case 1:
        du = -du;
        break;
      case 2:
        du = -du;
        dv = -dv;
        break;
      case 3:
        dv = -dv;
        break;
      }

      translateOnPlane(du * 0.5, dv * 0.5);
    }
  }
}

void ZAffineRect::setSizeWithMinCornerFixed(double width, double height)
{
  setSizeWithCornerFixed(width, height, 2);
  /*
  if (width >= 0.0 && height >= 0.0) {
    double du = width - m_width;
    double dv = height - m_height;
    setSize(width, height);
    translate(du * 0.5, dv * 0.5);
  }
  */
}

void ZAffineRect::setSizeWithMaxCornerFixed(double width, double height)
{
  setSizeWithCornerFixed(width, height, 0);
}


void ZAffineRect::translate(double dx, double dy, double dz)
{
  m_ap.translate(dx, dy, dz);
}

void ZAffineRect::translate(const ZPoint &dv)
{
  m_ap.translate(dv);
}

void ZAffineRect::translateOnPlane(double du, double dv)
{
  m_ap.translateOnPlane(du, dv);
}

void ZAffineRect::translateDepth(double d)
{
  m_ap.translateDepth(d);
}

void ZAffineRect::scale(double su, double sv)
{
  m_width *= su;
  m_height *= sv;
}

bool ZAffineRect::containsProjection(const ZPoint &pt) const
{
  ZPoint dp = pt - getCenter();

  double u = dp.dot(getV1());
  double halfWidth = getWidth() / 2.0;
  if (u < -halfWidth || u > halfWidth) {
    return false;
  }

  double v = dp.dot(getV2());
  double halfHeight = getHeight() / 2.0;
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
  double dist = m_ap.computeSignedDistance(pt);
  if (dist >= -d && dist < d) {
    return containsProjection(pt);
  }

  return false;
}

bool ZAffineRect::contains(const ZAffineRect &rect, double d) const
{
  if (m_ap == rect.m_ap) {
    return (getWidth() <= rect.getWidth()) && (getHeight() <= rect.getHeight());
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

  ZPoint xSpan = m_ap.getV1() * getWidth() / 2.0;
  ZPoint ySpan = m_ap.getV2() * getHeight() / 2.0;

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

ZPoint ZAffineRect::getMinCorner() const
{
  return getCorner(2);
}

ZPoint ZAffineRect::getMaxCorner() const
{
  return getCorner(0);
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
