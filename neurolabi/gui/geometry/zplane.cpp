#include "zplane.h"
#include <cmath>

ZPlane::ZPlane()
{

}

ZPlane::ZPlane(const ZPoint &v1, const ZPoint &v2)
{
  set(v1, v2);
}

void ZPlane::set(const ZPoint &v1, const ZPoint &v2)
{
  m_v1 = v1;
  m_v2 = v2;

  if (m_v1.isApproxOrigin() && m_v2.isApproxOrigin()) {
    m_v1.set(1, 0, 0);
    m_v2.set(0, 1, 0);
  } else if (m_v1.isApproxOrigin() || m_v2.isApproxOrigin() ||
             m_v1.isParallelTo(m_v2)) {
    double z = 1.0;
    if (m_v1.isApproxOrigin()) {
      m_v2.normalize();
      z = m_v2.getZ();
    } else {
      m_v1.normalize();
      z = m_v1.getZ();
    }

    if ((z > 1.0) || (std::fabs(z - 1.0) < ZPoint::MIN_DIST)) {
      //Make sure the value is valid
      z = 1.0;
    }
    double cosTheta = (z == 1.0) ? 0.0 : std::sqrt(1 - z * z);

    if (m_v1.isApproxOrigin()) {
      double sinPsi = (z == 1.0) ? 0.0 : -m_v2.getX() / cosTheta;
      double cosPsi = (z == 1.0) ? 1.0 : m_v2.getY() / cosTheta;
      m_v1.set(cosPsi, sinPsi, 0);
    } else {
      //    double sinTheta = -z;
      double sinPsi = (z == 1.0) ? 0.0 : m_v1.getY() / cosTheta;
      double cosPsi = (z == 1.0) ? 1.0 : m_v1.getX() / cosTheta;

      //Compute v2 based on rotation of (0, 1, 0)
      m_v2.set(-sinPsi, cosPsi, 0);
    }
  } else {
    m_v1.normalize();
    m_v2.normalize();
    if (!m_v1.isPendicularTo(m_v2)) {
      ZPoint normal = m_v1.cross(m_v2);
      m_v2 = normal.cross(m_v1);
      m_v2.normalize();
    }
  }
}

ZPoint ZPlane::getNormal() const
{
  return getV1().cross(getV2());
}

void ZPlane::invalidate()
{
  m_v1.set(0, 0, 0);
  m_v2.set(0, 0, 0);
}

bool ZPlane::isValid() const
{
  return m_v1.isUnitVector() && m_v2.isUnitVector() && m_v1.isPendicularTo(m_v2);
}

bool ZPlane::onSamePlane(const ZPlane &p) const
{
  if (isValid() && p.isValid()) {
    ZPoint normal = p.getNormal();
    if (normal.isPendicularTo(m_v1) && normal.isPendicularTo(m_v2)) {
      return true;
    }
  }

  return false;
}

ZPoint ZPlane::align(const ZPoint &pt) const
{
  return ZPoint(pt.dot(m_v1), pt.dot(m_v2), pt.dot(getNormal()));
}

ZPoint ZPlane::mapAligned(double u, double v) const
{
  return m_v1 * u + m_v2 * v;
}

bool ZPlane::contains(const ZPoint &pt) const
{
  if (pt.isApproxOrigin()) {
    return true;
  }

  return getNormal().isPendicularTo(pt);
}

double ZPlane::computeSignedDistance(double x, double y, double z) const
{
  return computeSignedDistance(ZPoint(x, y, z));
}

double ZPlane::computeSignedDistance(const ZPoint &pt) const
{
  ZPoint normal = getNormal();
  return pt.dot(normal);
}

bool ZPlane::approxEquals(const ZPlane &plane) const
{
  return m_v1.approxEquals(plane.m_v1) && m_v2.approxEquals(plane.m_v2);
}

bool ZPlane::operator== (const ZPlane &p) const
{
  return m_v1 == p.m_v1 && m_v2 == p.m_v2;
}

bool ZPlane::operator!= (const ZPlane &p) const
{
  return m_v1 != p.m_v1 || m_v2 != p.m_v2;
}

std::ostream& operator<<(std::ostream& stream, const ZPlane &p)
{
  stream << p.getV1() << " x " << p.getV2();

  return stream;
}
