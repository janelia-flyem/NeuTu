#ifndef ZAFFINEPLANE_H
#define ZAFFINEPLANE_H

#include "zplane.h"

class ZAffinePlane
{
public:
  ZAffinePlane();

  void setPlane(const ZPoint &v1, const ZPoint &v2);
  void setOffset(const ZPoint &offset);

  ZPoint getV1() const;
  ZPoint getV2() const;
  ZPoint getOffset() const;

  ZPoint getNormal() const;

private:
  ZPoint m_offset;
  ZPlane m_plane;
};

#endif // ZAFFINEPLANE_H
