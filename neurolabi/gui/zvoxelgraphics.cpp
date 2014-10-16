#include "zvoxelgraphics.h"
#include "zobject3d.h"
#include "tz_voxel_graphics.h"
#include "zlinesegment.h"
#include "tz_rastergeom.h"

static void addLineObject(
    ZObject3d *dst, const ZIntPoint &v1, const ZIntPoint &v2)
{
  ZObject3d *obj = ZVoxelGraphics::createLineObject(v1, v2);
  dst->append(*obj);

  delete obj;
}

static ZObject3d* createScanObject(const ZObject3d *baseLine1,
                                   const ZObject3d *baseLine2)
{
  ZObject3d *obj = new ZObject3d;

  size_t lineNumber1 = baseLine1->size();
  size_t lineNumber2 = baseLine2->size();

  if (lineNumber1 > lineNumber2) {
    for (size_t i = 0; i < lineNumber1; ++i) {
      int j = Raster_Line_Map(lineNumber1, lineNumber2, i);
      addLineObject(obj,
                 ZIntPoint(baseLine1->getX(i), baseLine1->getY(i), baseLine1->getZ(i)),
                 ZIntPoint(baseLine2->getX(j), baseLine2->getY(j), baseLine2->getZ(j)));
    }
  } else {
    for (size_t i = 0; i < lineNumber2; ++i) {
      int j = Raster_Line_Map(lineNumber2, lineNumber1, i);
      addLineObject(obj,
                 ZIntPoint(baseLine1->getX(j), baseLine1->getY(j), baseLine1->getZ(j)),
                 ZIntPoint(baseLine2->getX(i), baseLine2->getY(i), baseLine2->getZ(i)));
    }
  }

#if 0
  size_t lineNumber = std::min(lineNumber1, lineNumber2);
  for (size_t i = 0; i < lineNumber; ++i) {
    addLineObject(obj,
               ZIntPoint(baseLine1->x(i), baseLine1->y(i), baseLine1->z(i)),
               ZIntPoint(baseLine2->x(i), baseLine2->y(i), baseLine2->z(i)));
  }

  if (lineNumber1 > lineNumber) {
    ZIntPoint v(baseLine2->lastX(), baseLine2->lastY(), baseLine2->lastZ());
    for (size_t i = lineNumber; i < lineNumber1; ++i) {
      addLineObject(
            obj, v,
            ZIntPoint(baseLine1->x(i), baseLine1->y(i), baseLine1->z(i)));
    }
  } else if (lineNumber2 > lineNumber) {
    ZIntPoint v(baseLine1->lastX(), baseLine1->lastY(), baseLine1->lastZ());
    for (size_t i = lineNumber; i < lineNumber2; ++i) {
      addLineObject(
            obj, v,
            ZIntPoint(baseLine2->x(i), baseLine2->y(i), baseLine2->z(i)));
    }
  }
#endif

  return obj;
}

ZObject3d* ZVoxelGraphics::createLineObject(
    const ZIntPoint &v1, const ZIntPoint &v2)
{
  Voxel_t cv1;
  Voxel_t cv2;

  for (int i = 0; i < 3; ++i) {
    cv1[i] = v1[i];
    cv2[i] = v2[i];
  }

  Object_3d *cobj = Line_To_Object_3d(cv1, cv2);
  ZObject3d *obj = new ZObject3d;
  obj->setFromCObj(cobj);

  if (v1.getX() != obj->getX(0) || v1.getY() != obj->getY(0) ||
      v1.getZ() != obj->getZ(0)) {
    obj->reverse();
  }

  Kill_Object_3d(cobj);

  return obj;
}

ZObject3d* ZVoxelGraphics::createPlaneObject(
    const ZIntPoint &start, const ZPoint &vec1,
    double len1, const ZPoint &vec2, double len2)
{
  ZIntPoint lineEnd = start + (vec1 * len1).toIntPoint();
  ZObject3d *baseLine = createLineObject(start, lineEnd);

  lineEnd = start + (vec2 * len2).toIntPoint();
  ZObject3d *scanLine = createLineObject(start, lineEnd);

  ZObject3d *obj = new ZObject3d;
  obj->append(*scanLine);

  size_t lineNumber = baseLine->size();
  for (size_t i = 1; i < lineNumber; ++i) {
    int dx = baseLine->getX(i) - baseLine->getX(i - 1);
    int dy = baseLine->getY(i) - baseLine->getY(i - 1);
    int dz = baseLine->getZ(i) - baseLine->getZ(i - 1);

    scanLine->translate(dx, dy, dz);
    obj->append(*scanLine);
  }
  delete baseLine;
  delete scanLine;

  return obj;
}

ZObject3d* ZVoxelGraphics::createLineObject(const ZLineSegment &seg)
{
  return createLineObject(seg.getStartPoint().toIntPoint(),
                          seg.getEndPoint().toIntPoint());
}

ZObject3d* ZVoxelGraphics::createLineObject6c(
    const ZIntPoint &v1, const ZIntPoint &v2)
{
  ZObject3d *baseLine = createLineObject(v1, v2);
  size_t VoxelNumber = baseLine->size();

  ZObject3d *obj = new ZObject3d;
  obj->append(baseLine->getX(0), baseLine->getY(0), baseLine->getZ(0));

  for (size_t i = 1; i < VoxelNumber; ++i) {
    int x1 = baseLine->getX(i);
    int y1 = baseLine->getY(i);
    int z1 = baseLine->getZ(i);
    int x0 = obj->lastX();
    int y0 = obj->lastY();
    int z0 = obj->lastZ();
    int diffX = x1 - x0;
    int diffY = y1 - y0;
    int diffZ = z1 - z0;
    if (diffX != 0) {
      if (diffY != 0 || diffZ != 0) {
        obj->append(x0 + diffX, y0, z0);
        if (diffY != 0 && diffZ != 0) {
          obj->append(x0 + diffX, y0 + diffY, z0);
        }
      }
    } else if (diffY != 0 && diffZ != 0) {
      obj->append(x0, y0 + diffY, z0);
    }

    obj->append(x1, y1, z1);
  }

  return obj;
}

ZObject3d* ZVoxelGraphics::createLineObject6c(const ZLineSegment &seg)
{
  return createLineObject6c(seg.getStartPoint().toIntPoint(),
                            seg.getEndPoint().toIntPoint());
}

ZObject3d* ZVoxelGraphics::createPolylineObject(
    const std::vector<ZIntPoint> &polyline)
{
  if (polyline.empty()) {
    return NULL;
  }

  if (polyline.size() == 1) {
    return createLineObject(polyline[0], polyline[1]);
  }

  ZObject3d *obj = createLineObject(polyline[0], polyline[1]);
  for (size_t i = 1; i < polyline.size() - 1; ++i) {
    ZObject3d *line = createLineObject(polyline[i], polyline[i + 1]);
    obj->append(*line, 1);
    delete line;
  }

  return obj;
}

ZObject3d* ZVoxelGraphics::createPolylineObject6c(
    const std::vector<ZIntPoint> &polyline)
{
  if (polyline.empty()) {
    return NULL;
  }

  if (polyline.size() == 1) {
    return createLineObject(polyline[0], polyline[0]);
  }

  ZObject3d *obj = createLineObject6c(polyline[0], polyline[1]);
  for (size_t i = 1; i < polyline.size() - 1; ++i) {
    ZObject3d *line = createLineObject6c(polyline[i], polyline[i + 1]);
    obj->append(*line, 1);
    delete line;
  }

  return obj;
}

ZObject3d* ZVoxelGraphics::createPolyPlaneObject(
    const std::vector<ZIntPoint> &polyline, const ZPoint &vec2, double len2)
{
  ZObject3d *baseLine = createPolylineObject6c(polyline);

  ZIntPoint lineEnd = polyline[0] + (vec2 * len2).toIntPoint();
  ZObject3d *scanLine = createLineObject(polyline[0], lineEnd);

  ZObject3d *obj = new ZObject3d;
  obj->append(*scanLine);

  size_t lineNumber = baseLine->size();
  for (size_t i = 1; i < lineNumber; ++i) {
    int dx = baseLine->getX(i) - baseLine->getX(i - 1);
    int dy = baseLine->getY(i) - baseLine->getY(i - 1);
    int dz = baseLine->getZ(i) - baseLine->getZ(i - 1);

    scanLine->translate(dx, dy, dz);
    obj->append(*scanLine);
  }
  delete baseLine;
  delete scanLine;

  return obj;
}

ZObject3d* ZVoxelGraphics::createPolyPlaneObject(
    const std::vector<ZIntPoint> &polyline1,
    const std::vector<ZIntPoint> &polyline2)
{
  if (polyline1.empty() || polyline2.empty()) {
    return NULL;
  }

  ZObject3d *baseLine1 = createPolylineObject6c(polyline1);
  ZObject3d *baseLine2 = createPolylineObject6c(polyline2);

  ZObject3d *obj = createScanObject(baseLine1, baseLine2);

  delete baseLine1;
  delete baseLine2;

  return obj;
}

ZObject3d* ZVoxelGraphics::createTriangleObject(
    const ZIntPoint &pt1, const ZIntPoint &pt2, const ZIntPoint &pt3)
{
  ZObject3d* baseLine = createLineObject6c(pt2, pt3);
  ZObject3d *obj = new ZObject3d;
  size_t lineNumber = baseLine->size();
  for (size_t i = 0; i < lineNumber; ++i) {
    ZObject3d *scanLine = createLineObject(
          pt1, ZIntPoint(baseLine->getX(i), baseLine->getY(i), baseLine->getZ(i)));
    obj->append(*scanLine);
    delete scanLine;
  }

  delete baseLine;

  return obj;
}

ZObject3d* ZVoxelGraphics::createQuadrangleObject(
    const ZLineSegment &seg1, const ZLineSegment &seg2)
{
  return createQuadrangleObject(seg1.getStartPoint().toIntPoint(),
                                seg1.getEndPoint().toIntPoint(),
                                seg2.getStartPoint().toIntPoint(),
                                seg2.getEndPoint().toIntPoint());
}

ZObject3d* ZVoxelGraphics::createQuadrangleObject(
    const ZIntPoint &pt1, const ZIntPoint &pt2,
    const ZIntPoint &pt3, const ZIntPoint &pt4)
{
  ZObject3d *baseLine1 = createLineObject6c(pt1, pt2);
  ZObject3d *baseLine2 = createLineObject6c(pt3, pt4);

  ZObject3d *obj = createScanObject(baseLine1, baseLine2);

  delete baseLine1;
  delete baseLine2;

  return obj;
}
