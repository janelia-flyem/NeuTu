#ifndef ZVOXELGRAPHICS_H
#define ZVOXELGRAPHICS_H

#include "zintpoint.h"
#include "zpoint.h"

class ZObject3d;
class ZLineSegment;

namespace ZVoxelGraphics
{
ZObject3d* createLineObject(const ZIntPoint &v1, const ZIntPoint &v2);
ZObject3d* createLineObject(const ZLineSegment &seg);
/*!
 * \brief Create 6-connected line object
 */
ZObject3d* createLineObject6c(const ZIntPoint &v1, const ZIntPoint &v2);
ZObject3d* createLineObject6c(const ZLineSegment &seg);

ZObject3d* createPlaneObject(const ZIntPoint &start, const ZPoint &vec1,
                             double len1, const ZPoint &vec2, double len2);

ZObject3d* createPolylineObject(const std::vector<ZIntPoint> &polyline);
ZObject3d* createPolylineObject6c(const std::vector<ZIntPoint> &polyline);


ZObject3d* createPolyPlaneObject(const std::vector<ZIntPoint> &polyline,
                                 const ZPoint &vec2, double len2);
ZObject3d* createPolyPlaneObject(const std::vector<ZIntPoint> &polyline1,
                                 const std::vector<ZIntPoint> &polyline2);

/*!
 * \brief Create a disk object in 3D
 */
ZObject3d* createDiskObject(const ZPoint &center, double r, const ZPoint &norm);

/*!
 * \brief Create a triangle object in 3D.
 *
 * \a pt1 - \a pt2 - \a pt3 form a triangle.
 */
ZObject3d* createTriangleObject(const ZIntPoint &pt1, const ZIntPoint &pt2,
                                const ZIntPoint &pt3);

/*!
 * \brief Create a quadrangle object in 3D
 */
ZObject3d* createQuadrangleObject(const ZLineSegment &seg1,
                                 const ZLineSegment &seg2);

ZObject3d* createQuadrangleObject(const ZIntPoint &pt1, const ZIntPoint &pt2,
                                  const ZIntPoint &pt3, const ZIntPoint &pt4);


}

#endif // ZVOXELGRAPHICS_H
