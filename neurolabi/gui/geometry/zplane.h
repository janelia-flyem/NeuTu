#ifndef ZPLANE_H
#define ZPLANE_H

#include "zpoint.h"

class ZPlane
{
public:
  ZPlane();

  ZPoint getV1() const;
  ZPoint getV2() const;

private:
  ZPoint m_v1 = ZPoint(1, 0, 0);
  ZPoint m_v2 = ZPoint(0, 1, 0);
};

#endif // ZPLANE_H
