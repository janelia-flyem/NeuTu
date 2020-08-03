#include "zweightedpoint.h"

ZWeightedPoint::ZWeightedPoint()
{
}

ZWeightedPoint::ZWeightedPoint(const ZPoint &pt)
{
  set(pt.x(), pt.y(), pt.z());
}

ZWeightedPoint::ZWeightedPoint(double x, double y, double z, double w)
{
  set(x, y, z);
  setWeight(w);
}

void ZWeightedPoint::set(const ZPoint &pt, double w)
{
  set(pt);
  m_weight = w;
}

std::ostream &operator<<(std::ostream &stream,
                         const ZWeightedPoint &pt)
{
  stream << "(" << pt.x() << ", " << pt.y() << ", " << pt.z() << ", "
         << pt.weight() << ")";

  return stream;
}
