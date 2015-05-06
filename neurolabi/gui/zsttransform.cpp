#include "zsttransform.h"

#include <cmath>
#include <QPoint>
#include <QPointF>
#include <QRectF>

#include "zpoint.h"


ZStTransform::ZStTransform() : m_sx(1.0), m_sy(1.0), m_sz(1.0),
  m_dx(0.0), m_dy(0.0), m_dz(0.0)
{
}

void ZStTransform::setOffset(double dx, double dy, double dz)
{
  m_dx = dx;
  m_dy = dy;
  m_dz = dz;
}

void ZStTransform::setScale(double sx, double sy, double sz)
{
  m_sx = sx;
  m_sy = sy;
  m_sz = sz;
}

void ZStTransform::setOffset(double dx, double dy)
{
  m_dx = dx;
  m_dy = dy;
}

void ZStTransform::setScale(double sx, double sy)
{
  m_sx = sx;
  m_sy = sy;
}

double ZStTransform::getScale(NeuTube::EAxis axis) const
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return getSx();
  case NeuTube::Y_AXIS:
    return getSy();
  case NeuTube::Z_AXIS:
    return getSz();
  }

  return 1.0;
}

double ZStTransform::getOffset(NeuTube::EAxis axis) const
{
  switch (axis) {
  case NeuTube::X_AXIS:
    return getTx();
  case NeuTube::Y_AXIS:
    return getTy();
  case NeuTube::Z_AXIS:
    return getTz();
  }

  return 0.0;
}

ZPoint ZStTransform::getOffset() const
{
  return ZPoint(getTx(), getTy(), getTz());
}

bool ZStTransform::isIntTransform() const
{
  return isIntOffset() && isIntScale();
}

bool ZStTransform::isIntOffset() const
{
  return (std::ceil(getTx()) == getTx()) &&
      (std::ceil(getTy()) == getTy()) &&
      (std::ceil(getTz()) == getTz());
}

bool ZStTransform::isIntScale() const
{
  return (std::ceil(getSx()) == getSx()) &&
      (std::ceil(getSy()) == getSy()) &&
      (std::ceil(getSz()) == getSz());
}

bool ZStTransform::hasOffset() const
{
  return (getTx() != 0.0) || (getTy() != 0.0) || (getTz() != 0.0);
}

bool ZStTransform::hasScale() const
{
  return (getSx() != 1.0) || (getSy() != 1.0) || (getSz() != 1.0);
}

ZStTransform ZStTransform::getInverseTransform() const
{
  ZStTransform transform;
  transform.setOffset(
        -getTx() / getSx(), -getTy() / getSy(), -getTz() / getSz());
  transform.setScale(1.0 / getSx(), 1.0 / getSy(), 1.0 / getSz());

  return transform;
}

ZPoint ZStTransform::transform(const ZPoint &pt) const
{
  return ZPoint(transformX(pt.x()), transformY(pt.y()), transformZ(pt.z()));
}

QPointF ZStTransform::transform(const QPointF &pt) const
{
  return QPointF(transformX(pt.x()), transformY(pt.y()));
}

QPointF ZStTransform::transform(const QPoint &pt) const
{
  return QPointF(transformX(pt.x()), transformY(pt.y()));
}

QRectF ZStTransform::transform(const QRectF &rect) const
{
  return QRectF(transformX(rect.left()), transformY(rect.top()),
                rect.width() * getSx(), rect.height() * getSy());
}

bool ZStTransform::isIdentity() const
{
  return !hasOffset() && !hasScale();
}

void ZStTransform::estimate(const QRectF &input, const QRectF &output)
{
  if (input.isEmpty()) {
    m_sx = 1.0;
    m_sy = 1.0;
    m_dx = 0.0;
    m_dy = 0.0;
    m_sz = 1.0;
    m_dz = 0.0;
  } else {
    m_sx = output.width() / input.width();
    m_sy = output.height() / input.height();
    m_dx = output.left() - m_sx * input.left();
    m_dy = output.top() - m_sx * input.top();
    m_sz = 1.0;
    m_dz = 0.0;
  }
}
