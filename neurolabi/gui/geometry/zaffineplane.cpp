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
