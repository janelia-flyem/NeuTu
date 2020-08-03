#ifndef ZWEIGHTEDPOINT_H
#define ZWEIGHTEDPOINT_H

#include "geometry/zpoint.h"

class ZWeightedPoint : public ZPoint
{
public:
  ZWeightedPoint();
  explicit ZWeightedPoint(const ZPoint &pt);
  ZWeightedPoint(double x, double y, double z, double w);

  inline double weight() const { return m_weight; }
  inline void setWeight(double weight) { m_weight = weight; }

  using ZPoint::set;
  void set(const ZPoint &pt, double w);

  friend std::ostream& operator<<(std::ostream& stream,
                                  const ZWeightedPoint &pt);

private:
  double m_weight = 0.0;
};


#endif // ZWEIGHTEDPOINT_H
