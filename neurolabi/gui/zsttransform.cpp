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

void ZStTransform::addOffset(double dx, double dy)
{
  m_dx += dx;
  m_dy += dy;
}

void ZStTransform::addScaledOffset(double dx, double dy)
{
  if (m_sx !=  0.0) {
    m_dx += dx / m_sx;
  }

  if (m_sy != 0.0) {
    m_dy += dy / m_sy;
  }
}

void ZStTransform::setScale(double sx, double sy)
{
  m_sx = sx;
  m_sy = sy;
}

double ZStTransform::getScale(neutube::EAxis axis) const
{
  switch (axis) {
  case neutube::X_AXIS:
    return getSx();
  case neutube::Y_AXIS:
    return getSy();
  case neutube::Z_AXIS:
  case neutube::A_AXIS:
    return getSz();
  }

  return 1.0;
}

double ZStTransform::getOffset(neutube::EAxis axis) const
{
  switch (axis) {
  case neutube::X_AXIS:
    return getTx();
  case neutube::Y_AXIS:
    return getTy();
  case neutube::Z_AXIS:
  case neutube::A_AXIS:
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

ZStTransform ZStTransform::transform(const ZStTransform &transform) const
{
  ZStTransform t;
  t.setOffset(getTx() * transform.getSx() + transform.getTx(),
              getTy() * transform.getSy() + transform.getTy(),
              getTz() * transform.getSz() + transform.getTz());
  t.setScale(getSx() * transform.getSx(),
             getSy() * transform.getSy(),
             getSz() * transform.getSz());

  return t;
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

void ZStTransform::estimate(
    double source0, double source1, double target0, double target1,
    neutube::EAxis axis)
{
  double s = 0.0;
  double t = 0.0;
  if (source0 == source1) {
    s = 1.0;
    t = target0 - source0;
  } else {
    s = (target1 - target0) / (source1 - source0);
    t = target0 - s * source0;
  }

  switch (axis) {
  case neutube::X_AXIS:
    m_sx = s;
    m_dx = t;
    break;
  case neutube::Y_AXIS:
    m_sy = s;
    m_dy = t;
    break;
  case neutube::Z_AXIS:
  case neutube::A_AXIS:
    m_sz = s;
    m_dz = t;
    break;
  }
}
