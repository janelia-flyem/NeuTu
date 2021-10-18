#ifndef ZROTATOR_H
#define ZROTATOR_H

#include "zpoint.h"

class ZPlane;

class ZRotator
{
public:
  ZRotator(const ZPoint &axis, double angle);

  void set(const ZPoint &axis, double angle);
  void setAxis(const ZPoint &axis);
  void setAxis(double x, double y, double z);
  void setAngle(double angle);

  ZPoint rotate(const ZPoint &pt) const;
  ZPoint rotate(const ZPoint &pt, double angle) const;
  ZPlane rotate(const ZPlane &plane) const;
  ZPlane rotate(const ZPlane &plane, double angle) const;


private:
  ZPoint m_axis;
  double m_angle;
};

#endif // ZROTATOR_H
