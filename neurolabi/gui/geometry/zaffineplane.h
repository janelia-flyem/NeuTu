#ifndef ZAFFINEPLANE_H
#define ZAFFINEPLANE_H

#include "zplane.h"

class ZAffinePlane
{
public:
  ZAffinePlane();

  void set(const ZPoint &offset, const ZPoint &v1, const ZPoint &v2);
  void setPlane(const ZPoint &v1, const ZPoint &v2);
  void setOffset(const ZPoint &offset);

  ZPoint getV1() const;
  ZPoint getV2() const;
  ZPoint getOffset() const;

  ZPoint getNormal() const;

  ZPlane getPlane() const;

  bool onSamePlane(const ZAffinePlane &ap) const;
  bool contains(const ZPoint &pt) const;
  double computeSignedDistance(const ZPoint &pt) const;
  double computeSignedDistance(double x, double y, double z) const;

  ZPoint align(const ZPoint &pt) const;

private:
  ZPoint m_offset;
  ZPlane m_plane;
};

#endif // ZAFFINEPLANE_H
