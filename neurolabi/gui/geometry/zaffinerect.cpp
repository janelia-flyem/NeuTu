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
