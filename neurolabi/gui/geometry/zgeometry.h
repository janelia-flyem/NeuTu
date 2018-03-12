#ifndef ZGEOMETRY_H
#define ZGEOMETRY_H

#include <vector>
#include <utility>

#include "zgeo3dtransform.h"
#include "zgeo3dscalarfield.h"
#include "neutube_def.h"

class ZPoint;

namespace ZGeometry
{
/*!
 * \brief Transform a 3D field
 */
void transform(ZGeo3dScalarField *field, const ZGeo3dTransform &transform);
template <typename T>
void shiftSliceAxis(T &x, T &y, T &z, neutube::EAxis axis);
template <typename T>
void shiftSliceAxisInverse(T &x, T &y, T &z, neutube::EAxis axis);

std::vector<ZPoint> LineShpereIntersection(
    const ZPoint &lineStart, const ZPoint &lineNorm,
    const ZPoint &center, double r);
}

std::vector<std::pair<int, int> > LineToPixel(int x0, int y0, int x1, int y1);

template <typename T>
void ZGeometry::shiftSliceAxis(T &x, T &y, T &z, neutube::EAxis axis)
{
  switch (axis) {
  case neutube::X_AXIS:
//    std::swap(x, y);
    std::swap(x, z);
    break;
  case neutube::Y_AXIS:
    std::swap(y, z);
    break;
  case neutube::Z_AXIS:
    break;
  case neutube::A_AXIS:
    break;
  }
}

template <typename T>
void ZGeometry::shiftSliceAxisInverse(T &x, T &y, T &z, neutube::EAxis axis)
{
  switch (axis) {
  case neutube::X_AXIS:
    std::swap(x, z);
//    std::swap(x, y);
    break;
  case neutube::Y_AXIS:
    std::swap(y, z);
    break;
  case neutube::Z_AXIS:
    break;
  case neutube::A_AXIS:
    break;
  }
}

#endif // ZGEOMETRY_H
