#ifndef ZGEOMETRY_H
#define ZGEOMETRY_H

#include "zgeo3dtransform.h"
#include "zgeo3dscalarfield.h"
#include "neutube_def.h"

namespace ZGeometry
{
/*!
 * \brief Transform a 3D field
 */
void transform(ZGeo3dScalarField *field, const ZGeo3dTransform &transform);
template <typename T>
void shiftSliceAxis(T &x, T &y, T &z, NeuTube::EAxis axis);
template <typename T>
void shiftSliceAxisInverse(T &x, T &y, T &z, NeuTube::EAxis axis);
}

template <typename T>
void ZGeometry::shiftSliceAxis(T &x, T &y, T &z, NeuTube::EAxis axis)
{
  switch (axis) {
  case NeuTube::X_AXIS:
//    std::swap(x, y);
    std::swap(x, z);
    break;
  case NeuTube::Y_AXIS:
    std::swap(y, z);
    break;
  case NeuTube::Z_AXIS:
    break;
  }
}

template <typename T>
void ZGeometry::shiftSliceAxisInverse(T &x, T &y, T &z, NeuTube::EAxis axis)
{
  switch (axis) {
  case NeuTube::X_AXIS:
    std::swap(x, z);
//    std::swap(x, y);
    break;
  case NeuTube::Y_AXIS:
    std::swap(y, z);
    break;
  case NeuTube::Z_AXIS:
    break;
  }
}

#endif // ZGEOMETRY_H
