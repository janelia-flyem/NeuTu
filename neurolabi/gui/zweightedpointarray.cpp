#include "zweightedpointarray.h"
#include "zcuboid.h"

ZWeightedPointArray::ZWeightedPointArray()
{
}

Geo3d_Scalar_Field* ZWeightedPointArray::toScalarField() const
{
  if (empty()) {
    return NULL;
  }

  Geo3d_Scalar_Field *field = Make_Geo3d_Scalar_Field(size());

  for (size_t i = 0; i < size(); i++) {
    field->points[i][0] = (*this)[i].x();
    field->points[i][1] = (*this)[i].y();
    field->points[i][2] = (*this)[i].z();
    field->values[i] = (*this)[i].weight();
  }

  return field;
}

ZCuboid ZWeightedPointArray::getBoundBox() const
{
  ZCuboid boundBox;
  if (!empty()) {
    const ZPoint &pt = (*this)[0];
    boundBox.set(pt.x(), pt.y(), pt.z(), pt.x(), pt.y(), pt.z());
  }

  for (ZWeightedPointArray::const_iterator iter = begin(); iter != end(); ++iter) {
    boundBox.include(*iter);
  }

  return boundBox;
}

ZPoint ZWeightedPointArray::computeCentroid() const
{
  ZPoint centroid;
  if (!empty()) {
    double weight = 0.0;
    for (const_iterator iter = begin(); iter != end(); ++iter) {
      const ZWeightedPoint& pt = *iter;
      centroid += ZPoint(pt.x(), pt.y(), pt.z()) * pt.weight();
      weight += pt.weight();
    }

    if (weight > 0.0) {
      centroid /= weight;
    } else {
      for (const_iterator iter = begin(); iter != end(); ++iter) {
        const ZWeightedPoint& pt = *iter;
        centroid += ZPoint(pt.x(), pt.y(), pt.z());
      }
      centroid /= size();
    }
  }

  return centroid;
}

ZPoint ZWeightedPointArray::principalDirection() const
{
  Geo3d_Scalar_Field *field = toScalarField();

  double vec[3];

  Geo3d_Scalar_Field_Ort(field, vec, NULL);

  Kill_Geo3d_Scalar_Field(field);

  return ZPoint(vec[0], vec[1], vec[2]);
}

void ZWeightedPointArray::append(const ZWeightedPoint &pt)
{
  push_back(pt);
}

void ZWeightedPointArray::append(double x, double y, double z, double weight)
{
  push_back(ZWeightedPoint(x, y, z, weight));
}

void ZWeightedPointArray::append(const ZPoint &center, double weight)
{
  append(center.x(), center.y(), center.z(), weight);
}
