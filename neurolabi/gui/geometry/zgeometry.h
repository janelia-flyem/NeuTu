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
 * \brief Compute intersectin ratio for a line segment intersecting with a plane.
 *
 * Note that the intersection ratio between the line segment
 * (\a lineStart, \a lineEnd) and \a plane can indicate a point outside of the
 * line segment which the ratio is less than 0 or greate than 1. The ratio is 0
 * when both \a lineStart and \a lineEnd is on the plane (approximately).
 */
double ComputeIntersection(
    const ZPlane &plane, const ZPoint &lineStart, const ZPoint &lineEnd);

double ComputeIntersection(const ZPlane &plane, const ZLineSegment &seg);
double ComputeIntersection(const ZAffinePlane &plane, const ZLineSegment &seg);

/*!
 * \brief Compute intersection point between a plane and a line segment
 *
 * It returns an invalid point if there is no intersection. It returns
 * \a lineStart if both \a lineStart and \a lineEnd are on the plane
 * (approximately).
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

/*!
 * \brief A fast way of estimating intersection
 *
 * \a box intersects with \a rect approximately iff the components of the vector
 * from the rectangle center to the box center is less than
 * (v1: width/2 + v1Range, v2: height/2 + v2Range, n: normalRange).
 */
bool IntersectsApprox(
    const ZAffineRect &rect, const ZCuboid &box,
    double normalRange, double v1Range, double v2Range);

void EstimateBoxRange(
    const ZCuboid &box, const ZPoint &a1, const ZPoint &a2, const ZPoint &a3,
    double &r1, double &r2, double &r3);

bool IntersectsApprox(
    const ZAffineRect &rect, const ZCuboid &box);

ZIntCuboid GetIntBoundBox(const ZAffineRect &rect);

/*!
 * \brief Convert a float range to an int range
 *
 * It returns the minimal int range to contain [\a x0, \a x1) or x0 if x0 == x1.
 *
 * Since we use corner mapping to map between the integer and float system,
 * \a x0 is mapped to floor(\a x0), and \a x1 is mapped to x1 - 1 if it is an
 * integer number or floor(\a x1) if it is not.
 *
 * The behavior of this function is not defined if x0 > x1.
 */
std::pair<int, int> ToIntRange(double x0, double x1);

/*!
 * \brief Convert a float segment to an int segment
 *
 * It returns an integer segment that contains a segment centered at \a cx with
 * lenght \a length. The returned segment is (cx, length) with cx as the right
 * int center.
 */
std::pair<int, int> ToIntSegAtLeftCenter(double cx, double length);

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
