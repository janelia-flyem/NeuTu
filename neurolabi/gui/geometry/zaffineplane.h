#ifndef ZAFFINEPLANE_H
#define ZAFFINEPLANE_H

#include "zplane.h"

class ZAffinePlane
{
public:
  ZAffinePlane();

  ZPoint getV1() const;
  ZPoint getV2() const;
  ZPoint getOffset() const;

private:
  ZPoint m_offset;
  ZPlane m_plane;
};

#endif // ZAFFINEPLANE_H
