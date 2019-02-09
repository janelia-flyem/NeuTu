#ifndef ZGEOMETRY_H
#define ZGEOMETRY_H

#include <vector>
#include <utility>

#include "zgeo3dtransform.h"
#include "zgeo3dscalarfield.h"
#include "common/neutube_def.h"

class ZPoint;
class ZAffineRect;
class ZIntCuboid;
class ZIntPoint;

namespace zgeom
{

std::vector<ZAffineRect> Partition(const ZAffineRect &rect, int row, int col);

/*!
 * \brief Transform a 3D field
 */
void transform(ZGeo3dScalarField *field, const ZGeo3dTransform &transform);
template <typename T>
void shiftSliceAxis(T &x, T &y, T &z, neutu::EAxis axis);
template <typename T>
void shiftSliceAxisInverse(T &x, T &y, T &z, neutu::EAxis axis);

std::vector<ZPoint> LineShpereIntersection(
    const ZPoint &lineStart, const ZPoint &lineNorm,
    const ZPoint &center, double r);

bool IsPerpendicular(const ZPoint &p1, const ZPoint &p2);

bool IsSameAffinePlane(const ZPoint &c1, const ZPoint &v1x, const ZPoint &v1y,
                       const ZPoint &c2, const ZPoint &v2x, const ZPoint &v2y);
std::vector<std::pair<int, int> > LineToPixel(int x0, int y0, int x1, int y1);

ZIntCuboid MakeSphereBox(const ZIntPoint &center, int radius);

int GetZoomScale(int zoom);
int GetZoomLevel(int scale);

void CopyToArray(const ZIntPoint &pt, int v[]);

}


template <typename T>
void zgeom::shiftSliceAxis(T &x, T &y, T &z, neutu::EAxis axis)
{
  switch (axis) {
  case neutu::EAxis::X:
//    std::swap(x, y);
    std::swap(x, z);
    break;
  case neutu::EAxis::Y:
    std::swap(y, z);
    break;
  case neutu::EAxis::Z:
    break;
  case neutu::EAxis::ARB:
    break;
  }
}

template <typename T>
void zgeom::shiftSliceAxisInverse(T &x, T &y, T &z, neutu::EAxis axis)
{
  switch (axis) {
  case neutu::EAxis::X:
    std::swap(x, z);
//    std::swap(x, y);
    break;
  case neutu::EAxis::Y:
    std::swap(y, z);
    break;
  case neutu::EAxis::Z:
    break;
  case neutu::EAxis::ARB:
    break;
  }
}

#endif // ZGEOMETRY_H
