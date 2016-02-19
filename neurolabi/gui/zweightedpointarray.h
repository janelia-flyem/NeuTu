#ifndef ZWEIGHTEDPOINTARRAY_H
#define ZWEIGHTEDPOINTARRAY_H

#include <vector>
#include "tz_geo3d_scalar_field.h"
#include "zweightedpoint.h"

class ZCuboid;

class ZWeightedPointArray : public std::vector<ZWeightedPoint>
{
public:
  ZWeightedPointArray();

public:
  void append(const ZWeightedPoint &pt);
  void append(double x, double y, double z, double weight);
  void append(const ZPoint &center, double weight);

  Geo3d_Scalar_Field *toScalarField() const;
  ZPoint principalDirection() const;
  ZPoint computeCentroid() const;
  ZCuboid getBoundBox() const;
};

#endif // ZWEIGHTEDPOINTARRAY_H
