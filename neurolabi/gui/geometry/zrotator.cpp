#include "zrotator.h"

#include <cmath>

#include "zplane.h"

ZRotator::ZRotator(const ZPoint &axis, double angle)
{
  set(axis, angle);
}

void ZRotator::set(const ZPoint &axis, double angle)
{
  setAxis(axis);
  m_angle = angle;
}

void ZRotator::setAxis(const ZPoint &axis)
{
  m_axis = axis.getNormalized();
  if (m_axis.isApproxOrigin()) {
    m_axis = ZPoint(1, 0, 0);
  }
}

void ZRotator::setAxis(double x, double y, double z)
{
  setAxis(ZPoint(x, y, z));
}

void ZRotator::setAngle(double angle)
{
  m_angle = angle;
}

ZPoint ZRotator::rotate(const ZPoint &pt) const
{
  return rotate(pt, m_angle);
}

ZPoint ZRotator::rotate(const ZPoint &pt, double angle) const
{
  double cosAngle = std::cos(angle);
  double sinAngle = std::sin(angle);

  double dot = m_axis.dot(pt) * (1.0 - cosAngle);
  double u = m_axis.getX();
  double v = m_axis.getY();
  double w = m_axis.getZ();

  double x = pt.getX();
  double y = pt.getY();
  double z = pt.getZ();

  return ZPoint(
        u * dot + x * cosAngle + (-w * y +  v * z) * sinAngle,
        v * dot + y * cosAngle + (w * x - u * z) * sinAngle,
        w * dot + z * cosAngle + (-v * x + u * y) * sinAngle);
}

ZPlane ZRotator::rotate(const ZPlane &plane) const
{
  return rotate(plane, m_angle);
}

ZPlane ZRotator::rotate(const ZPlane &plane, double angle) const
{
  return ZPlane(rotate(plane.getV1(), angle), rotate(plane.getV2(), angle));
}
