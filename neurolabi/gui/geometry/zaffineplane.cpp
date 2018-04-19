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
