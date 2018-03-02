#include "zarbsliceviewparam.h"
#include <QRect>

ZArbSliceViewParam::ZArbSliceViewParam()
{
}

void ZArbSliceViewParam::setCenter(const ZIntPoint &center)
{
  m_center = center;
}

void ZArbSliceViewParam::setCenter(int x, int y, int z)
{
  m_center.set(x, y, z);
}

void ZArbSliceViewParam::setPlane(const ZPoint &v1, const ZPoint &v2)
{
  m_v1 = v1;
  m_v2 = v2;
  m_v1.normalize();
  m_v2.normalize();
}

void ZArbSliceViewParam::setSize(int width, int height)
{
  m_width = width;
  m_height = height;
}

bool ZArbSliceViewParam::isValid() const
{
  return m_v1.isPendicularTo(m_v2) && m_width > 0 && m_height > 0;
}

QRect ZArbSliceViewParam::getViewPort() const
{
  QRect rect;
  rect.moveCenter(QPoint(m_center.getX(), m_center.getY()));
  rect.setSize(QSize(m_width, m_height));

  return rect;
}

int ZArbSliceViewParam::getX() const
{
  return m_center.getX();
}

int ZArbSliceViewParam::getY() const
{
  return m_center.getY();
}

int ZArbSliceViewParam::getZ() const
{
  return m_center.getZ();
}

ZIntPoint ZArbSliceViewParam::getCenter() const
{
  return m_center;
}

ZPoint ZArbSliceViewParam::getPlaneV1() const
{
  return m_v1;
}

ZPoint ZArbSliceViewParam::getPlaneV2() const
{
  return m_v2;
}

int ZArbSliceViewParam::getWidth() const
{
  return m_width;
}

int ZArbSliceViewParam::getHeight() const
{
  return m_height;
}

bool ZArbSliceViewParam::contains(const ZArbSliceViewParam &param)
{
  if (m_v1.approxEquals(param.m_v1) && m_v2.approxEquals(param.m_v2) &&
      m_center.getZ() == param.m_center.getZ()) {
    return getViewPort().contains(param.getViewPort());
  }

  return false;
}
