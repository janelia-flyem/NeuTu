#include "zaffineplane.h"

#include <sstream>

ZAffinePlane::ZAffinePlane()
{

}

ZAffinePlane::ZAffinePlane(
    const ZPoint &offset, const ZPoint &v1, const ZPoint &v2)
{
  set(offset, v1, v2);
}

ZPoint ZAffinePlane::getV1() const
{
  return m_plane.getV1();
}

ZPoint ZAffinePlane::getV2() const
{
  return m_plane.getV2();
}

ZPoint ZAffinePlane::getOffset() const
{
  return m_offset;
}

void ZAffinePlane::set(const ZPoint &offset, const ZPoint &v1, const ZPoint &v2)
{
  setOffset(offset);
  setPlane(v1, v2);
}

ZPoint ZAffinePlane::getNormal() const
{
  return m_plane.getNormal();
}

void ZAffinePlane::setPlane(const ZPoint &v1, const ZPoint &v2)
{
  m_plane.set(v1, v2);
}

void ZAffinePlane::setOffset(const ZPoint &offset)
{
  m_offset = offset;
}

ZPlane ZAffinePlane::getPlane() const
{
  return m_plane;
}

void ZAffinePlane::translate(double dx, double dy, double dz)
{
  m_offset.translate(dx, dy, dz);
}

void ZAffinePlane::translate(const ZPoint &dv)
{
  m_offset.translate(dv);
}

void ZAffinePlane::translateDepth(double d)
{
  translate(getNormal() * d);
}

double ZAffinePlane::computeSignedDistance(const ZPoint &pt) const
{
  ZPoint newPt = pt - m_offset;
  return m_plane.computeSignedDistance(newPt);
}

double ZAffinePlane::computeSignedDistance(double x, double y, double z) const
{
  return computeSignedDistance(ZPoint(x, y, z));
}

bool ZAffinePlane::contains(const ZPoint &pt) const
{
  return m_plane.contains(pt - m_offset);
}

bool ZAffinePlane::onSamePlane(const ZAffinePlane &ap) const
{
  if (ap.m_plane.onSamePlane(m_plane)) {
    return contains(ap.getOffset());
  }

  return false;
}

ZPoint ZAffinePlane::align(const ZPoint &pt) const
{
  return m_plane.align(pt - m_offset);
}

std::string ZAffinePlane::toString() const
{
  std::ostringstream stream;
  stream << *this;
  return stream.str();
}

bool ZAffinePlane::approxEquals(const ZAffinePlane &plane) const
{
  return m_plane.approxEquals(plane.m_plane) &&
      m_offset.approxEquals(plane.m_offset);
}

std::ostream& operator<<(std::ostream& stream, const ZAffinePlane &p)
{
  stream << p.m_plane << " @ " << p.m_offset;

  return stream;
}
