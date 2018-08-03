#include "zplane.h"

ZPlane::ZPlane()
{

}

ZPoint ZPlane::getV1() const
{
  return m_v1;
}

ZPoint ZPlane::getV2() const
{
  return m_v2;
}

void ZPlane::set(const ZPoint &v1, const ZPoint &v2)
{
  m_v1 = v1;
  m_v2 = v2;
}

ZPoint ZPlane::getNormal() const
{
  return getV1().cross(getV2());
}
