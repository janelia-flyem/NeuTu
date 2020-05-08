#include "zarbsliceviewparam.h"
#include <QRect>
#include <cmath>

#include "geometry/zaffineplane.h"
#include "geometry/zaffinerect.h"

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

void ZArbSliceViewParam::resize(int width, int height)
{
  setSize(width, height);
}

bool ZArbSliceViewParam::isValid() const
{
  return m_v1.isPendicularTo(m_v2) && m_width > 0 && m_height > 0;
}

QRect ZArbSliceViewParam::getViewPort() const
{
  QRect rect;
  rect.setSize(QSize(m_width, m_height));
  rect.moveCenter(QPoint(m_center.getX(), m_center.getY()));

  return rect;
}

int ZArbSliceViewParam::getX0() const
{
  return getViewPort().left();
}

int ZArbSliceViewParam::getY0() const
{
  return getViewPort().top();
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

ZPoint ZArbSliceViewParam::getPlaneNormal() const
{
  return m_v1.cross(m_v2);
}

int ZArbSliceViewParam::getWidth() const
{
  return m_width;
}

int ZArbSliceViewParam::getHeight() const
{
  return m_height;
}

ZAffinePlane ZArbSliceViewParam::getAffinePlane() const
{
  ZAffinePlane ap;
  ap.setOffset(getCenter().toPoint());
  ap.setPlane(getPlaneV1(), getPlaneV2());

  return ap;
}

ZAffineRect ZArbSliceViewParam::getAffineRect() const
{
  ZAffineRect rect;
  rect.set(getCenter().toPoint(), getPlaneV1(), getPlaneV2(), getWidth(), getHeight());

  return rect;
}

void ZArbSliceViewParam::move(double dx, double dy, double dz)
{
  if (isValid()) {
    ZPoint dp = getPlaneV1() * dx + getPlaneV2() * dy + getPlaneNormal() * dz;
    m_center += dp.toIntPoint();
  }
}

bool ZArbSliceViewParam::operator ==(const ZArbSliceViewParam &param) const
{
  return (m_v1.approxEquals(param.m_v1) && m_v2.approxEquals(param.m_v2) &&
          m_center== param.m_center && m_width == param.m_width &&
          m_height == param.m_height);
}

bool ZArbSliceViewParam::operator !=(const ZArbSliceViewParam &param) const
{
  return !(*this == param);
}

bool ZArbSliceViewParam::hasSamePlaneCenter(const ZArbSliceViewParam &param) const
{
  return (m_v1.approxEquals(param.m_v1) && m_v2.approxEquals(param.m_v2) &&
          m_center== param.m_center);
}

bool ZArbSliceViewParam::contains(const ZArbSliceViewParam &param)
{
  if (isSamePlane(param)) {
    if (param.getViewPort().isEmpty()) {
      return true;
    } else {
      return getViewPort().contains(param.getViewPort());
    }
  }

  /*
  if (m_v1.approxEquals(param.m_v1) && m_v2.approxEquals(param.m_v2)) {
    ZPoint dc = (m_center - param.m_center).toPoint();
    ZPoint normal = m_v1.cross(m_v2);
    if (std::fabs(dc.dot(normal)) <= ZPoint::MIN_DIST) {
      if (param.getViewPort().isEmpty()) {
        return true;
      } else {
        return getViewPort().contains(param.getViewPort());
      }
    }
  }
  */

  return false;
}

bool ZArbSliceViewParam::isSamePlane(const ZArbSliceViewParam &param) const
{
  if (m_v1.approxEquals(param.m_v1) && m_v2.approxEquals(param.m_v2)) {
    ZPoint dc = (m_center - param.m_center).toPoint();
    ZPoint normal = m_v1.cross(m_v2);
    if (dc.isApproxOrigin() && dc.isPendicularTo(normal)) {
      return true;
    }
  }

  return false;
}
