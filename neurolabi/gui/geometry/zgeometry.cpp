#include "zgeometry.h"
#include <cmath>

#include"common/utilities.h"
#include "common/math.h"

#include "zpoint.h"
#include "zaffinerect.h"
#include "zintcuboid.h"
#include "zintpoint.h"
#include "zcuboid.h"
#include "zlinesegment.h"

std::vector<ZAffineRect> zgeom::Partition(
    const ZAffineRect &rect, int row, int col)
{
  std::vector<ZAffineRect> result;

  double subwidth = rect.getWidth() / col;
  double subheight = rect.getHeight() / row;
//  int heightRemainder = rect.getHeight() % row;

  double currentOffsetY = -rect.getHeight() / 2;
  for (int i = 0; i < row; ++i) {
    double currentOffsetX = -rect.getWidth() / 2;
    double height = subheight;
    /*
    if (heightRemainder > 0) {
      ++height;
      --heightRemainder;
    }
    int widthRemainder = rect.getWidth() % col;
    */

    for (int j = 0; j < col; ++j) {
      double width = subwidth;
      /*
      if (widthRemainder > 0) {
        ++width;
        --widthRemainder;
      }
      */

      ZAffineRect subrect;

      ZPoint center = rect.getV1() * (currentOffsetX + width / 2) +
          rect.getV2() * (currentOffsetY + height / 2) + rect.getCenter();
      subrect.set(center, rect.getV1(), rect.getV2(), width, height);

      result.push_back(subrect);

      currentOffsetX += width;
    }
    currentOffsetY += height;
  }

  return result;
}

std::vector<ZAffineRect> zgeom::IntPartition(
    const ZAffineRect &rect, int row, int col)
{
  std::vector<ZAffineRect> result;

  int rectWidth = neutu::iround(rect.getWidth());
  int rectHeight = neutu::iround(rect.getHeight());

  int subwidth = rectWidth / col;
  int subheight = rectHeight / row;
  int heightRemainder = rectHeight % row;

  double currentOffsetY = -rectHeight / 2;
  for (int i = 0; i < row; ++i) {
    double currentOffsetX = -rectWidth / 2;
    int height = subheight;
    if (heightRemainder > 0) {
      ++height;
      --heightRemainder;
    }
    int widthRemainder = rectWidth % col;

    for (int j = 0; j < col; ++j) {
      int width = subwidth;
      if (widthRemainder > 0) {
        ++width;
        --widthRemainder;
      }

      ZAffineRect subrect;

      ZPoint center = rect.getV1() * (currentOffsetX + width / 2) +
          rect.getV2() * (currentOffsetY + height / 2) + rect.getCenter();
      subrect.set(center, rect.getV1(), rect.getV2(), width, height);

      result.push_back(subrect);

      currentOffsetX += width;
    }
    currentOffsetY += height;
  }

  return result;
}

void zgeom::Transform(ZGeo3dScalarField *field,
                     const ZGeo3dTransform &transform)
{
  transform.transform(field->getRawPointArray(), field->getPointNumber());
}

bool IsPerpendicular(const ZPoint &p1, const ZPoint &p2)
{
  return p1.isPendicularTo(p2);
}

std::vector<ZPoint> zgeom::LineShpereIntersection(
    const ZPoint &lineStart, const ZPoint &lineNorm,
    const ZPoint &center, double r)
{
  std::vector<ZPoint> result;

  ZPoint dc = lineStart - center;

  double b = dc.dot(lineNorm);
  double c = dc.lengthSqure() - r * r;
  double q = b * b - c;

  if (q >= 0) {
    double d = -b - std::sqrt(q);
    result.push_back(lineStart + lineNorm * d);
    d = -b + std::sqrt(q);
    result.push_back(lineStart + lineNorm * d);
  }

  return result;
}

std::vector<std::pair<int, int> > zgeom::LineToPixel(int x0, int y0, int x1, int y1)
{
  std::vector<std::pair<int, int>> result;

  int dx = x1 - x0;
  int dy = y1 - y0;
  int x = x0;
  int y = y0;

  if (dy >= 0) {
    if (dx >= 0 && (dx >= dy)) {
      int p = 2 * dy - dx;
      while (x <= x1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          ++y;
          p = p + 2 * dy - 2 * dx;
        } else {
          result.emplace_back(x, y);
          p = p + 2 * dy;
        }
        ++x;
      }
    } else if (dx >= 0) {
      int p = 2 * dx - dy;
      while (y <= y1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          ++x;
          p = p + 2 * dx - 2 * dy;
        } else {
          result.emplace_back(x, y);
          p = p + 2 * dx;
        }
        ++y;
      }
    } else if (dx < 0 && -dx >= dy) {
      int p = 2 * dy + dx;
      while (x >= x1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          ++y;
          p = p + 2 * dy + 2 * dx;
        } else {
          result.emplace_back(x, y);
          p = p + 2 * dy;
        }
        --x;
      }
    } else if (dx < 0) {
      int p = -2 * dx - dy;
      while (y <= y1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          --x;
          p = p - 2 * dx - 2 * dy;
        } else {
          result.emplace_back(x, y);
          p = p - 2 * dx;
        }
        ++y;
      }
    }
  } else {
    if (dx >= 0 && (dx >= -dy)) {
      int p = -2 * dy - dx;
      while (x <= x1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          --y;
          p = p - 2 * dy - 2 * dx;
        } else {
          result.emplace_back(x, y);
          p = p - 2 * dy;
        }
        ++x;
      }
    } else if (dx >= 0) {
      int p = 2 * dx + dy;
      while (y >= y1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          ++x;
          p = p + 2 * dx + 2 * dy;
        } else {
          result.emplace_back(x, y);
          p = p + 2 * dx;
        }
        --y;
      }
    } else if (dx < 0 && -dx >= -dy) {
      int p = -2 * dy + dx;
      while (x >= x1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          --y;
          p = p - 2 * dy + 2 * dx;
        } else {
          result.emplace_back(x, y);
          p = p - 2 * dy;
        }
        --x;
      }
    } else if (dx < 0) {
      int p = -2 * dx + dy;
      while (y >= y1) {
        if (p >= 0) {
          result.emplace_back(x,y);
          --x;
          p = p - 2 * dx + 2 * dy;
        } else {
          result.emplace_back(x, y);
          p = p - 2 * dx;
        }
        --y;
      }
    }
  }

//  result.emplace_back(x1, y1);

  return result;
}

bool zgeom::IsSameAffinePlane(
    const ZPoint &c1, const ZPoint &v1x, const ZPoint &v1y,
    const ZPoint &c2, const ZPoint &v2x, const ZPoint &v2y)
{
  ZAffinePlane ap1;
  ap1.setOffset(c1);
  ap1.setPlane(v1x, v1y);

  ZAffinePlane ap2;
  ap2.setOffset(c2);
  ap2.setPlane(v2x, v2y);

  return ap1.onSamePlane(ap2);
}

ZIntCuboid zgeom::MakeSphereBox(const ZIntPoint &center, int radius)
{
  return ZIntCuboid(center - radius, center  + radius);
}

int zgeom::GetZoomScale(int zoom)
{
  switch (zoom) {
  case 0:
    return 1;
  case 1:
    return 2;
  case 2:
    return 4;
  case 3:
    return 8;
  case 4:
    return 16;
  case 5:
    return 32;
  case 6:
    return 64;
  default:
    break;
  }

  int scale = 1;
  while (zoom--) {
    scale *= 2;
  }

  return scale;
}

int zgeom::GetZoomLevel(int scale)
{
  switch (scale) {
  case 1:
    return 0;
  case 2:
    return 1;
  case 4:
    return 2;
  case 8:
    return 3;
  case 16:
    return 4;
  case 32:
    return 5;
  case 64:
    return 6;
  default:
    break;
  }

  int zoom = 0;
  while ((scale /= 2) > 0) {
    ++zoom;
  }

  return zoom;
}

void zgeom::CopyToArray(const ZIntPoint &pt, int v[])
{
  v[0] = pt.getX();
  v[1] = pt.getY();
  v[2] = pt.getZ();
}

double zgeom::ComputeIntersection(
    const ZPlane &plane, const ZPoint &lineStart, const ZPoint &lineEnd)
{
  ZPoint v0 = plane.align(lineStart);
  ZPoint v1 = plane.align(lineEnd);

  double lambda = 0.0;
  double sz = v0.getZ() - v1.getZ();
  if (std::fabs(sz) > ZPoint::MIN_DIST) {
    lambda = v0.getZ() / sz;
  }

  return lambda;
}

double zgeom::ComputeIntersection(const ZPlane &plane, const ZLineSegment &seg)
{
  return ComputeIntersection(plane, seg.getStartPoint(), seg.getEndPoint());
}

double zgeom::ComputeIntersection(const ZAffinePlane &plane, const ZLineSegment &seg)
{
  return ComputeIntersection(plane.getPlane(), seg - plane.getOffset());
}

ZPoint zgeom::ComputeIntersectionPoint(
    const ZPlane &plane, const ZPoint &lineStart, const ZPoint &lineEnd)
{
  return ComputeIntersectionPoint(plane, ZLineSegment(lineStart, lineEnd));
}

ZPoint zgeom::ComputeIntersectionPoint(
    const ZPlane &plane, const ZLineSegment &seg)
{
  double lambda = ComputeIntersection(plane, seg);

  ZPoint intersection;
  intersection.invalidate();
  if (lambda > 0.0 && lambda < 1.0) {
    intersection = seg.getIntercept(lambda);
  }

  return intersection;

  /*
  ZPoint v0 = plane.align(seg.getStartPoint());
  ZPoint v1 = plane.align(seg.getEndPoint());

  ZPoint intersection;
  intersection.invalidate();

  if ((v0.getZ() > 0.0 && v1.getZ() < 0.0) ||
      (v0.getZ() < 0.0 && v1.getZ() > 0.0)) {
    double lambda = std::fabs(v0.getZ()) /
        (std::fabs(v0.getZ()) + std::fabs(v1.getZ()));
    intersection = seg.getIntercept(lambda);
  }

  return intersection;
  */
}

ZPoint zgeom::ComputeIntersectionPoint(
    const ZAffinePlane &plane, const ZLineSegment &seg)
{
  ZPoint v0 = seg.getStartPoint();
  ZPoint v1 = seg.getEndPoint();
  v0 -= plane.getOffset();
  v1 -= plane.getOffset();
  double lambda = ComputeIntersection(plane.getPlane(), seg);

  ZPoint intersection;
  intersection.invalidate();
  if (lambda > 0.0 && lambda < 1.0) {
    intersection = seg.getIntercept(lambda);
  }

  return intersection;
}

bool zgeom::Intersects(
    const ZAffineRect &rect, double x, double y, double z, double r)
{
  ZPoint pt = rect.getAffinePlane().align(ZPoint(x, y, z));

  double halfWidth = rect.getWidth() * 0.5;
  double halfHeight = rect.getHeight() * 0.5;

  return neutu::WithinOpenRange(pt.x(), -halfWidth - r, halfWidth + r) &&
      neutu::WithinOpenRange(pt.y(), -halfHeight - r, halfHeight + r) &&
      neutu::WithinOpenRange(pt.z(), -r, r);
}

bool zgeom::Intersects(const ZAffineRect &rect, const ZLineSegment &seg)
{
  ZPoint v0 = rect.getAffinePlane().align(seg.getStartPoint());
  ZPoint v1 = rect.getAffinePlane().align(seg.getEndPoint());

  ZPoint intersection = zgeom::ComputeIntersectionPoint(
        ZPlane(ZPoint(1, 0, 0), ZPoint(0, 1, 0)), ZLineSegment(v0, v1));
  if (intersection.isValid()) {
    double halfWidth = rect.getWidth() * 0.5;
    double halfHeight = rect.getHeight() * 0.5;
    return neutu::WithinCloseRange(intersection.x(), -halfWidth, halfWidth) &&
        neutu::WithinCloseRange(intersection.y(), -halfHeight, halfHeight);
  }

  return false;
}

bool zgeom::Intersects(const ZAffineRect &r1, const ZAffineRect &r2)
{
  for (int index = 0; index <= 3; ++index) {
    if (zgeom::Intersects(r1, r2.getSide(index))) {
      return true;
    }
    if (zgeom::Intersects(r2, r1.getSide(index))) {
      return true;
    }
  }

  return false;
}

ZIntCuboid zgeom::GetIntBoundBox(const ZAffineRect &rect)
{
  ZIntCuboid box;
  for (int i = 0; i < 4; ++i) {
    ZPoint corner = rect.getCorner(i);
    box.join(corner);
  }
  return box;
}

std::pair<int, int> zgeom::ToIntRange(double x0, double x1)
{
  std::pair<int, int> result;
  result.first = neutu::ifloor(x0);
  result.second = neutu::ifloor(x1);
  if ((double(result.second) == x1) && (result.first != result.second)) {
    --result.second;
  }

  return result;
}

std::pair<int, int> zgeom::ToIntSegAtLeftCenter(double cx, double length)
{
  double halfLength = length * 0.5;
  std::pair<int, int> range = ToIntRange(cx - halfLength, cx + halfLength);

  int intLength = range.second - range.first + 1;
  return std::pair<int, int>(
        range.first + intLength / 2, intLength);
}

namespace {

ZAffineRect get_face(const ZCuboid &box, int index)
{
  ZAffineRect rect;
  switch (index) {
  case 0:
    rect.setCenter((box.getCorner(0) + box.getCorner(3)) * 0.5);
    rect.setPlane(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    rect.setSize(neutu::iround(box.width()), neutu::iround(box.height()));
    break;
  case 1:
    rect.setCenter((box.getCorner(4) + box.getCorner(7)) * 0.5);
    rect.setPlane(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    rect.setSize(neutu::iround(box.width()), neutu::iround(box.height()));
    break;
  case 2:
    rect.setCenter(((box.getCorner(0) + box.getCorner(6)) * 0.5));
    rect.setPlane(ZPoint(0, 0, 1), ZPoint(0, 1, 0));
    rect.setSize(neutu::iround(box.depth()), neutu::iround(box.height()));
    break;
  case 3:
    rect.setCenter(((box.getCorner(1) + box.getCorner(7)) * 0.5));
    rect.setPlane(ZPoint(0, 0, 1), ZPoint(0, 1, 0));
    rect.setSize(neutu::iround(box.depth()), neutu::iround(box.height()));
    break;
  case 4:
    rect.setCenter(((box.getCorner(0) + box.getCorner(5)) * 0.5));
    rect.setPlane(ZPoint(1, 0, 0), ZPoint(0, 0, 1));
    rect.setSize(neutu::iround(box.width()), neutu::iround(box.depth()));
    break;
  case 5:
    rect.setCenter(((box.getCorner(2) + box.getCorner(7)) * 0.5));
    rect.setPlane(ZPoint(1, 0, 0), ZPoint(0, 0, 1));
    rect.setSize(neutu::iround(box.width()), neutu::iround(box.depth()));
    break;
  default:
    break;
  }

  return rect;
}

}

bool zgeom::Intersects(const ZAffineRect &rect, const ZCuboid &box)
{
  if (rect.isEmpty()) {
    return false;
  }

  if (box.isValid()) {
    for (int i = 0; i < 4; ++i) {
      if(box.contains(rect.getCorner(i))) {
        return true;
      }
    }

    for (int i = 0; i < 6; ++i) {
      ZAffineRect r = get_face(box, i);
      if (Intersects(rect, r)) {
        return true;
      }
    }
  }

  return false;
}

bool zgeom::Intersects(const ZAffineRect &rect, const ZIntCuboid &box)
{
  return Intersects(rect, ZCuboid::FromIntCuboid(box));
}

void zgeom::raster::ForEachNeighbor(
    int x, int y, int z, int nsx, int nsy, int nsz,
    std::function<void(int,int,int)> f)
{
  for (int k = -nsz; k <= nsz; ++k) {
    for (int j = -nsy; j <= nsy; ++j) {
      for (int i = -nsx; i <= nsx; ++i) {
        if (i != 0 || j != 0 || k != 0) {
          f(x + i, y + j, z + k);
        }
      }
    }
  }
}

template<>
void zgeom::raster::ForEachNeighbor<1>(
    int x, int y, int z, std::function<void(int,int,int)> f)
{
  f(x - 1, y, z);
  f(x + 1, y, z);
  f(x, y - 1, z);
  f(x, y + 1, z);
  f(x, y, z - 1);
  f(x, y, z + 1);
}

template<>
void zgeom::raster::ForEachNeighbor<2>(
    int x, int y, int z, std::function<void(int,int,int)> f)
{
  ForEachNeighbor<1>(x, y, z, f);

  f(x - 1, y - 1, z);
  f(x + 1, y - 1, z);
  f(x - 1, y + 1, z);
  f(x + 1, y + 1, z);
  f(x - 1, y, z -1);
  f(x + 1, y, z - 1);
  f(x - 1, y, z + 1);
  f(x + 1, y, z + 1);
  f(x, y - 1, z - 1);
  f(x, y + 1, z - 1);
  f(x, y - 1, z + 1);
  f(x, y + 1, z + 1);
}

template<>
void zgeom::raster::ForEachNeighbor<3>(
    int x, int y, int z, std::function<void(int,int,int)> f)
{
  ForEachNeighbor<2>(x, y, z, f);

  f(x - 1, y - 1, z - 1);
  f(x + 1, y - 1, z - 1);
  f(x - 1, y + 1, z - 1);
  f(x + 1, y + 1, z - 1);
  f(x - 1, y - 1, z + 1);
  f(x + 1, y - 1, z + 1);
  f(x - 1, y + 1, z + 1);
  f(x + 1, y + 1, z + 1);
}
