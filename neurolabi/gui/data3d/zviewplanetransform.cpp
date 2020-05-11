#include "zviewplanetransform.h"

#include <algorithm>

#include "geometry/zpoint.h"
#include "zjsonobject.h"

ZViewPlaneTransform::ZViewPlaneTransform()
{

}

void ZViewPlaneTransform::set(double dx, double dy, double s)
{
  m_dx = dx;
  m_dy = dy;
  setScale(s);
}

/*
QTransform ZViewPlaneTransform::getPainterTransform() const
{
  QTransform transform;
  transform.setMatrix(m_s, 0, 0, 0, m_s, 0, m_dx, m_dy, 1);

  return  transform;
}
*/

double ZViewPlaneTransform::getTx() const
{
  return m_dx;
}

double ZViewPlaneTransform::getTy() const
{
  return m_dy;
}

double ZViewPlaneTransform::getScale() const
{
  return m_s;
}

void ZViewPlaneTransform::translateTransform(
    double u, double v, double a, double b)
{
  m_dx = a - u * m_s;
  m_dy = b - v * m_s;
}

void ZViewPlaneTransform::translateTransform(
    const neutu::geom2d::Point &src, const neutu::geom2d::Point &dst)
{
  translateTransform(src.getX(), src.getY(), dst.getX(), dst.getY());
}

void ZViewPlaneTransform::setScale(double s)
{
  m_s = s;
  clipScale();
}

double ZViewPlaneTransform::getScaleStep() const
{
  return 1.1;
//  return 1.0 + std::min(10.0, 1.0 / m_s);
}

void ZViewPlaneTransform::clipScale()
{
  if (m_s > m_maxScale) {
    m_s = m_maxScale;
  } else if (m_s < m_minScale) {
    m_s = m_minScale;
  }
}

void ZViewPlaneTransform::incScale()
{
  setScale(getScale() * getScaleStep());
}

void ZViewPlaneTransform::decScale()
{
  setScale(getScale() / getScaleStep());
}

void ZViewPlaneTransform::setOffset(double tx, double ty)
{
  m_dx = tx;
  m_dy = ty;
}

void ZViewPlaneTransform::centerFit(
    double cu, double cv, double srcWidth, double srcHeight,
    double dstWidth, double dstHeight)
{
  if (srcWidth > 0.0 && srcHeight > 0.0) {
    setScale(std::min(dstWidth / srcWidth, dstHeight / srcHeight));

    double dstX = dstWidth / 2.0;
    double dstY = dstHeight / 2.0;
    translateTransform(cu, cv, dstX, dstY);
  }
}

void ZViewPlaneTransform::centerFit(
    const neutu::geom2d::Point &center, double srcWidth, double srcHeight,
    double dstWidth, double dstHeight)
{
  centerFit(
        center.getX(), center.getY(), srcWidth, srcHeight, dstWidth, dstHeight);
}

void ZViewPlaneTransform::centerFit(
    const neutu::geom2d::Rectangle &src,  const neutu::geom2d::Rectangle &dst)
{
  if (src.isValid()) {
    setScale(std::min(dst.getWidth() / src.getWidth(),
                      dst.getHeight() / src.getHeight()));

    translateTransform(src.getCenter(), dst.getCenter());
  }
}

void ZViewPlaneTransform::centerFit(
    double srcWidth, double srcHeight, double dstWidth, double dstHeight)
{
  centerFit(srcWidth / 2.0, srcHeight / 2.0, srcWidth, srcHeight,
            dstWidth, dstHeight);
}

void ZViewPlaneTransform::setScaleFixingOriginal(
    double s, double u, double v)
{
  double a = u;
  double b = v;
  transform(&a, &b);

  setScale(s);
  translateTransform(u, v, a, b);
}

void ZViewPlaneTransform::setScaleFixingMapped(double s, double a, double b)
{
  double u = a;
  double v = b;
  inverseTransform(&u, &v);
  setScale(s);
  translateTransform(u, v, a, b);
}

void ZViewPlaneTransform::setMinScale(double s)
{
  m_minScale = s;
}

double ZViewPlaneTransform::getMinScale() const
{
  return m_minScale;
}

void ZViewPlaneTransform::setMaxScale(double s)
{
  m_maxScale = s;
}

double ZViewPlaneTransform::getMaxScale() const
{
  return m_maxScale;
}

bool ZViewPlaneTransform::canZoomIn() const
{
  return getScale() < getMaxScale();
}

bool ZViewPlaneTransform::canZoomOut() const
{
  return getScale() > getMinScale();
}

neutu::geom2d::Point ZViewPlaneTransform::transform(double x, double y) const
{
  return neutu::geom2d::Point(x * m_s + m_dx, y * m_s + m_dy);
}

neutu::geom2d::Point ZViewPlaneTransform::transform(
    const neutu::geom2d::Point &pt) const
{
  return transform(pt.getX(), pt.getY());
}

void ZViewPlaneTransform::transform(double *x, double *y) const
{
  *x = *x * m_s + m_dx;
  *y = *y * m_s + m_dy;
}

ZPoint ZViewPlaneTransform::transform(const ZPoint &pt) const
{
  double x = pt.getX();
  double y = pt.getY();
  transform(&x, &y);

  return ZPoint(x, y, pt.getZ());
}

neutu::geom2d::Rectangle ZViewPlaneTransform::transform(
    const neutu::geom2d::Rectangle &rect) const
{
  neutu::geom2d::Rectangle newRect;
  newRect.setMinCorner(transform(rect.getMinCorner()));
  newRect.setMaxCorner(transform(rect.getMaxCorner()));

  return newRect;
}

void ZViewPlaneTransform::inverseTransform(double *x, double *y) const
{
  *x = (*x - m_dx) / m_s;
  *y = (*y - m_dy) / m_s;
}

ZPoint ZViewPlaneTransform::inverseTransform(const ZPoint &pt) const
{
  double x = pt.getX();
  double y = pt.getY();
  inverseTransform(&x, &y);

  return ZPoint(x, y, pt.getZ());
}

neutu::geom2d::Point
ZViewPlaneTransform::inverseTransform(const neutu::geom2d::Point &pt) const
{
  double x = pt.getX();
  double y = pt.getY();
  inverseTransform(&x, &y);

  return neutu::geom2d::Point(x, y);
}

neutu::geom2d::Rectangle ZViewPlaneTransform::inverseTransform(
    const neutu::geom2d::Rectangle &rect) const
{
  return neutu::geom2d::Rectangle(inverseTransform(rect.getMinCorner()),
                                  inverseTransform(rect.getMaxCorner()));
}

ZJsonObject ZViewPlaneTransform::toJsonObject() const
{
  ZJsonObject obj;

  obj.setEntry("s", m_s);
  obj.setEntry("t", std::vector<double>({m_dx, m_dy}));

  return obj;
}

bool ZViewPlaneTransform::operator==(const ZViewPlaneTransform &t) const
{
  return m_dx == t.m_dx && m_dy == t.m_dy && m_s == t.m_s;
}

std::ostream& operator<< (
      std::ostream &stream, const ZViewPlaneTransform &t)
{
  stream << "x " << t.m_s << " + " << "(" << t.m_dx << ", " << t.m_dy << ")";

  return stream;
}

