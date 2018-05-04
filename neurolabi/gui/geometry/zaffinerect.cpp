#include "zaffinerect.h"

ZAffineRect::ZAffineRect()
{

}

void ZAffineRect::set(
    const ZPoint &offset, const ZPoint &v1, const ZPoint &v2,
    int width, int height)
{
  m_ap.setOffset(offset);
  m_ap.setPlane(v1, v2);
  m_width = width;
  m_height = height;
}

int ZAffineRect::getWidth() const
{
  return m_width;
}

int ZAffineRect::getHeight() const
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

void ZAffineRect::setPlane(const ZPoint &v1, const ZPoint &v2)
{
  m_ap.setPlane(v1, v2);
}

void ZAffineRect::setSize(int width, int height)
{
  m_width = width;
  m_height = height;
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
