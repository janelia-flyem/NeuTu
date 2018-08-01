#include "zaffineplane.h"

ZAffinePlane::ZAffinePlane()
{

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
