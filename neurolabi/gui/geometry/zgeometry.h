#ifndef ZGEOMETRY_H
#define ZGEOMETRY_H

#include <vector>
#include <utility>
#include <functional>
#include <cmath>

#include "zgeo3dtransform.h"
#include "zgeo3dscalarfield.h"
#include "common/neutudefs.h"

class ZPoint;
class ZAffineRect;
class ZIntCuboid;
class ZIntPoint;
class ZCuboid;
class ZPlane;
class ZLineSegment;
class ZAffinePlane;

namespace zgeom
{

std::vector<ZAffineRect> Partition(const ZAffineRect &rect, int row, int col);
std::vector<ZAffineRect> IntPartition(const ZAffineRect &rect, int row, int col);

/*!
 * \brief Transform a 3D field
 */
void Transform(ZGeo3dScalarField *field, const ZGeo3dTransform &transform);
template <typename T>
void ShiftSliceAxis(T &x, T &y, T &z, neutu::EAxis axis);
template <typename T>
void ShiftSliceAxisInverse(T &x, T &y, T &z, neutu::EAxis axis);

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

/*!
 * \brief Compute intersectin ratio for a line segment intersecting with a plane
 */
double ComputeIntersection(
    const ZPlane &plane, const ZPoint &lineStart, const ZPoint &lineEnd);

double ComputeIntersection(const ZPlane &plane, const ZLineSegment &seg);
double ComputeIntersection(const ZAffinePlane &plane, const ZLineSegment &seg);

/*!
 * \brief Compute intersection point between a plane and a line segment
 *
 * It returns an invalid point if there is no intersection
 */
ZPoint ComputeIntersectionPoint(
    const ZPlane &plane, const ZPoint &lineStart, const ZPoint &lineEnd);

ZPoint ComputeIntersectionPoint(
    const ZPlane &plane, const ZLineSegment &seg);


ZPoint ComputeIntersectionPoint(
    const ZAffinePlane &plane, const ZLineSegment &seg);

bool Intersects(
    const ZAffineRect &rect, double x, double y, double z, double r);
bool Intersects(const ZAffineRect &rect, const ZCuboid &box);
bool Intersects(const ZAffineRect &rect, const ZIntCuboid &box);
bool Intersects(const ZAffineRect &rect, const ZLineSegment &seg);
bool Intersects(const ZAffineRect &r1, const ZAffineRect &r2);

ZIntCuboid GetIntBoundBox(const ZAffineRect &rect);

namespace raster {

void ForEachNeighbor(
    int x, int y, int z, int nsx, int nsy, int nsz,
    std::function<void(int,int,int)> f);

template<int N>
void ForEachNeighbor(int x, int y, int z, std::function<void(int,int,int)> f);

template<>
void ForEachNeighbor<1>(
    int x, int y, int z, std::function<void(int,int,int)> f);

template<>
void ForEachNeighbor<2>(
    int x, int y, int z, std::function<void(int,int,int)> f);

template<>
void ForEachNeighbor<3>(
    int x, int y, int z, std::function<void(int,int,int)> f);

extern template
void ForEachNeighbor<1>(
    int x, int y, int z, std::function<void(int,int,int)> f);

extern template
void ForEachNeighbor<2>(
    int x, int y, int z, std::function<void(int,int,int)> f);

extern template
void ForEachNeighbor<3>(
    int x, int y, int z, std::function<void(int,int,int)> f);
}

}


template <typename T>
void zgeom::ShiftSliceAxis(T &x, T &y, T &z, neutu::EAxis axis)
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
void zgeom::ShiftSliceAxisInverse(T &x, T &y, T &z, neutu::EAxis axis)
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
