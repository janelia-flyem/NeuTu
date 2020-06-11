#ifndef ZGEOMETRYTEST_H
#define ZGEOMETRYTEST_H

#include "ztestheader.h"
#include "geometry/zaffinerect.h"
#include "geometry/zgeometry.h"
#include "geometry/zplane.h"
#include "geometry/zaffineplane.h"
#include "geometry/zlinesegment.h"
#include "geometry/zcuboid.h"
#include "geometry/zintpoint.h"

#ifdef _USE_GTEST_
TEST(ZGeometry, Util)
{
  ASSERT_EQ(int(0), zgeom::GetZoomLevel(1));
  ASSERT_EQ(1, zgeom::GetZoomLevel(2));
  ASSERT_EQ(2, zgeom::GetZoomLevel(4));
  ASSERT_EQ(3, zgeom::GetZoomLevel(8));
  ASSERT_EQ(4, zgeom::GetZoomLevel(16));
  ASSERT_EQ(5, zgeom::GetZoomLevel(32));
  ASSERT_EQ(6, zgeom::GetZoomLevel(64));
  ASSERT_EQ(7, zgeom::GetZoomLevel(128));
  ASSERT_EQ(8, zgeom::GetZoomLevel(256));
  ASSERT_EQ(8, zgeom::GetZoomLevel(500));

}

TEST(ZGeometry, ZPlane)
{
  {
    ZPlane plane;
    ASSERT_TRUE(plane.isValid());
  }

  {
    ZPlane plane(ZPoint(0, 0, 0), ZPoint(0, 0, 0));
    ASSERT_TRUE(plane.isValid());
  }

  {
    ZPlane plane(ZPoint(1, 0, 0), ZPoint(0, 1, 1));
    ASSERT_TRUE(plane.isValid());

    plane.set(ZPoint(1, 0, 0), ZPoint(0, 0, 1));
    ASSERT_TRUE(plane.isValid());

    plane.set(ZPoint(1, 0, 0), ZPoint(0, 1, 0.1));
    ASSERT_TRUE(plane.isValid());

    plane.set(ZPoint(1, 0, 0), ZPoint(0, 0, 0));
    ASSERT_TRUE(plane.isValid());
    ASSERT_TRUE(plane.getV2().approxEquals(ZPoint(0, 1, 0)));

    plane.set(ZPoint(0, 0, 0), ZPoint(0, 1, 0));
    ASSERT_TRUE(plane.isValid());
    ASSERT_TRUE(plane.getV1().approxEquals(ZPoint(1, 0, 0)));

    plane.set(ZPoint(1, 0, 0), ZPoint(1, 0, 0));
    ASSERT_TRUE(plane.isValid());

    plane.set(ZPoint(1, 1, 0), ZPoint(0, 0, 0));
    ASSERT_TRUE(plane.isValid());

    plane.set(ZPoint(0, 1, 1), ZPoint(0, 1, 10));
    ASSERT_TRUE(plane.isValid());

    plane.set(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    ASSERT_TRUE(plane.getNormal().approxEquals(ZPoint(0, 0, 1)));

    ASSERT_EQ(plane, plane);
    ASSERT_NE(ZPlane(ZPoint(0, 0, 1), ZPoint(0, 1, 0)), plane);
    ASSERT_NE(ZPlane(ZPoint(1, 0, 0), ZPoint(0, 0, 1)), plane);

    ASSERT_TRUE(plane.approxEquals(ZPlane(ZPoint(1, 0, 0), ZPoint(0, 1, 0))));
    ASSERT_FALSE(plane.approxEquals(ZPlane(ZPoint(0, 1, 0), ZPoint(1, 0, 0))));

//    ASSERT_TRUE()
  }

  {
    ZPlane plane;
    plane.set(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    ASSERT_TRUE(plane.getNormal().approxEquals(ZPoint(0, 0, 1)));

    ASSERT_TRUE(plane.contains(ZPoint(0, 0, 0)));
    ASSERT_TRUE(plane.contains(ZPoint(1, 0, 0)));
    ASSERT_TRUE(plane.contains(ZPoint(1, 1, 0)));

    plane.set(ZPoint(0, 1, 0), ZPoint(0, 0, 1));
    ASSERT_TRUE(plane.contains(ZPoint(0, 0, 0)));
    ASSERT_TRUE(plane.contains(ZPoint(0, 1, 0)));
    ASSERT_TRUE(plane.contains(ZPoint(0, 1, 1)));

    ZPlane p2;
    p2.set(ZPoint(0, 1, 1), ZPoint(0, 1, 10));
    ASSERT_TRUE(plane.onSamePlane(p2));
  }

  {
    ZPlane plane;
    plane.set(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    ASSERT_DOUBLE_EQ(0.0, plane.computeSignedDistance(0, 0, 0));
    ASSERT_DOUBLE_EQ(1.0, plane.computeSignedDistance(0, 0, 1));
    ASSERT_DOUBLE_EQ(1.0, plane.computeSignedDistance(1, 0, 1));
    ASSERT_DOUBLE_EQ(-1.0, plane.computeSignedDistance(1, 0, -1));
  }

  {
    ZPlane plane;
    plane.set(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    ZPoint pt = plane.align(ZPoint(1, 2, 3));
    ASSERT_DOUBLE_EQ(1.0, pt.getX());
    ASSERT_DOUBLE_EQ(2.0, pt.getY());
    ASSERT_DOUBLE_EQ(3.0, pt.getZ());

    plane.set(ZPoint(0, 0, 1), ZPoint(1, 0, 0));
    pt = plane.align(ZPoint(1, 2, 3));
    ASSERT_DOUBLE_EQ(3.0, pt.getX());
    ASSERT_DOUBLE_EQ(1.0, pt.getY());
    ASSERT_DOUBLE_EQ(2.0, pt.getZ());
  }
}

TEST(ZGeometry, ZAffinePlane)
{
  {
    ZAffinePlane ap;
    ap.set(ZPoint(0, 0, 0), ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    ASSERT_TRUE(ap.contains(ZPoint(0, 0, 0)));
    ASSERT_FALSE(ap.contains(ZPoint(0, 0, 1)));

    ap.setOffset(ZPoint(1, 2, 3));
    ASSERT_DOUBLE_EQ(0.0, ap.computeSignedDistance(1, 2, 3));
    ASSERT_DOUBLE_EQ(1.0, ap.computeSignedDistance(1, 2, 4));
    ASSERT_DOUBLE_EQ(-1.0, ap.computeSignedDistance(1, 2, 2));

    ap.set(ZPoint(1, 0, 0), ZPoint(0, 1, 0), ZPoint(0, 0, 1));
    ASSERT_TRUE(ap.contains(ZPoint(1, 0, 0)));
    ASSERT_TRUE(ap.contains(ZPoint(1, 0, 1)));
    ASSERT_FALSE(ap.contains(ZPoint(0, 0, 1)));

    ZAffinePlane ap2;
    ap2.set(ZPoint(1, 2, 3), ZPoint(0, 1, 0), ZPoint(0, 0, 1));
    ASSERT_TRUE(ap.onSamePlane(ap2));

    ASSERT_TRUE(
          ap2.approxEquals(
            ZAffinePlane(ZPoint(1, 2, 3), ZPoint(0, 1, 0), ZPoint(0, 0, 1))));
    ASSERT_FALSE(
          ap2.approxEquals(
            ZAffinePlane(ZPoint(1.1, 2, 3), ZPoint(0, 1, 0), ZPoint(0, 0, 1))));

    ZPoint pt = ap2.align(ZPoint(2, 4, 6));
    ASSERT_DOUBLE_EQ(2.0, pt.getX());
    ASSERT_DOUBLE_EQ(3.0, pt.getY());
    ASSERT_DOUBLE_EQ(1.0, pt.getZ());

    ap2.setOffset(ZPoint(2, 0, 0));
    ASSERT_FALSE(ap.onSamePlane(ap2));
  }
}

TEST(ZGeometry, Intersect)
{
  {
    ZPlane plane(ZPoint(1, 0, 0), ZPoint(0, 1, 0));

    ASSERT_DOUBLE_EQ(0.0, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 0), ZPoint(0, 0, 0)));
    ASSERT_DOUBLE_EQ(0.0, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 0), ZPoint(0, 0, 1)));
    ASSERT_DOUBLE_EQ(0.0, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 0), ZPoint(1, 1, 0)));
    ASSERT_DOUBLE_EQ(1.0, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 1), ZPoint(0, 0, 0)));

    ASSERT_DOUBLE_EQ(0.5, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, -1), ZPoint(0, 0, 1)));
    ASSERT_DOUBLE_EQ(0.5, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, -1), ZPoint(10, 10, 1)));
    ASSERT_DOUBLE_EQ(0.1, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, -1), ZPoint(0, 0, 9)));
    ASSERT_DOUBLE_EQ(0.5, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 1), ZPoint(0, 0, -1)));
    ASSERT_DOUBLE_EQ(0.5, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 1), ZPoint(10, 10, -1)));
    ASSERT_DOUBLE_EQ(0.9, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 9), ZPoint(0, 0, -1)));
    ASSERT_DOUBLE_EQ(2.0, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 2), ZPoint(0, 0, 1)));
    ASSERT_DOUBLE_EQ(-1.0, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, 1), ZPoint(0, 0, 2)));
    ASSERT_DOUBLE_EQ(-1.0, zgeom::ComputeIntersection(
                       plane, ZPoint(0, 0, -1), ZPoint(0, 0, -2)));
  }

  {
    ZAffineRect rect;
    rect.setCenter(ZPoint(0, 0, 0));
    rect.setPlane(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    rect.setSize(16, 32);
    ASSERT_TRUE(zgeom::Intersects(rect, 0, 0, 0, 1));
    ASSERT_FALSE(zgeom::Intersects(rect, 0, 0, 1, 1));
    ASSERT_TRUE(zgeom::Intersects(rect, 0, 0, 0, 1));
    ASSERT_TRUE(zgeom::Intersects(rect, 5, 10, 0, 1));
    ASSERT_FALSE(zgeom::Intersects(rect, 9, 10, 0, 1));
    ASSERT_FALSE(zgeom::Intersects(rect, 5, 17, 0, 1));
    ASSERT_TRUE(zgeom::Intersects(rect, 0, 0, 0.5, 1));
    ASSERT_TRUE(zgeom::Intersects(rect, 5, 10, -0.5, 1));
    ASSERT_TRUE(zgeom::Intersects(
                  rect, ZLineSegment(ZPoint(0, 0, -1), ZPoint(0, 0, 1))));
    ASSERT_TRUE(zgeom::Intersects(
                  rect, ZLineSegment(
                    ZPoint(-100, -100, -1), ZPoint(100, 100, 1))));
    ASSERT_FALSE(zgeom::Intersects(
                   rect, ZLineSegment(ZPoint(7, 0, -1), ZPoint(10, 0, 1))));
  }

  {
    ZAffineRect rect;
    rect.setCenter(ZPoint(0, 0, 0));
    rect.setPlane(ZPoint(0, 1, 0), ZPoint(0, 0, 1));
    rect.setSize(16, 32);
    ASSERT_TRUE(zgeom::Intersects(rect, 0, 0, 0, 1));
    ASSERT_FALSE(zgeom::Intersects(rect, 1, 0, 0, 1));
    ASSERT_TRUE(zgeom::Intersects(rect, 0, 0, 0, 1));
    ASSERT_TRUE(zgeom::Intersects(rect, 0, 5, 10, 1));
    ASSERT_FALSE(zgeom::Intersects(rect, 0, 9, 10, 1));
    ASSERT_FALSE(zgeom::Intersects(rect, 0, 5, 17, 1));
    ASSERT_TRUE(zgeom::Intersects(rect, 0.5, 0, 0, 1));
    ASSERT_TRUE(zgeom::Intersects(rect, -0.5, 5, 10, 1));

    ASSERT_TRUE(zgeom::Intersects(
                  rect, ZLineSegment(ZPoint(-1, 0, 0), ZPoint(1, 0, 0))));
    ASSERT_TRUE(zgeom::Intersects(
                  rect, ZLineSegment(
                    ZPoint(-1, -100, -100), ZPoint(1, 100, 100))));
    ASSERT_FALSE(zgeom::Intersects(
                   rect, ZLineSegment(ZPoint(-1, 7, 0), ZPoint(1, 10, 0))));
  }

  {
    ZPlane plane;
    plane.set(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    ZPoint pt = zgeom::ComputeIntersectionPoint(
          plane, ZLineSegment(ZPoint(0, 0, -1), ZPoint(0, 0, 1)));
    ASSERT_TRUE(pt.approxEquals(ZPoint(0, 0, 0))) << pt;
    pt = zgeom::ComputeIntersectionPoint(
              plane, ZLineSegment(ZPoint(0, 0, -1), ZPoint(0, 0, 3)));
    ASSERT_TRUE(pt.approxEquals(ZPoint(0, 0, 0)));

    pt = zgeom::ComputeIntersectionPoint(
              plane, ZLineSegment(ZPoint(1, 2, -1), ZPoint(3, 4, 1)));
    ASSERT_TRUE(pt.approxEquals(ZPoint(2, 3, 0)));

    pt = zgeom::ComputeIntersectionPoint(
          plane, ZLineSegment(ZPoint(0, 0, 0), ZPoint(1, 0, 0)));
    ASSERT_FALSE(pt.isValid());

    pt = zgeom::ComputeIntersectionPoint(
          plane, ZLineSegment(ZPoint(0, 0, 1), ZPoint(1, 0, 2)));
    ASSERT_FALSE(pt.isValid());
  }

  {
    ZPlane plane;
    plane.set(ZPoint(0, 1, 0), ZPoint(0, 0, 1));
    ZPoint pt = zgeom::ComputeIntersectionPoint(
          plane, ZLineSegment(ZPoint(-1, 0, 0), ZPoint(1, 0, 0)));
    ASSERT_TRUE(pt.approxEquals(ZPoint(0, 0, 0))) << pt;
    pt = zgeom::ComputeIntersectionPoint(
              plane, ZLineSegment(ZPoint(-1, 0, 0), ZPoint(3, 0, 0)));
    ASSERT_TRUE(pt.approxEquals(ZPoint(0, 0, 0)));

    pt = zgeom::ComputeIntersectionPoint(
              plane, ZLineSegment(ZPoint(-1, 1, 2), ZPoint(1, 3, 4)));
    ASSERT_TRUE(pt.approxEquals(ZPoint(0, 2, 3)));

    pt = zgeom::ComputeIntersectionPoint(
          plane, ZLineSegment(ZPoint(0, 0, 0), ZPoint(1, 0, 0)));
    ASSERT_FALSE(pt.isValid());

    pt = zgeom::ComputeIntersectionPoint(
          plane, ZLineSegment(ZPoint(1, 0, 0), ZPoint(2, 1, 0)));
    ASSERT_FALSE(pt.isValid());
  }

  {
    ZAffineRect rect;
    rect.set(ZPoint(1, 2, 4), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 10, 20);

    ZLineSegment seg(1, 2, 0, 1, 2, 10);
    ASSERT_TRUE(zgeom::Intersects(rect, seg));
    ASSERT_FALSE(zgeom::Intersects(rect, ZLineSegment(20, 30, 0, 20, 30, 10)));

    {
      ZAffineRect rect2;
      rect2.set(ZPoint(1, 2, 4), ZPoint(0, 1, 0), ZPoint(0, 0, 1), 30, 40);
      ASSERT_TRUE(zgeom::Intersects(rect, rect2));
    }

    {
      ZAffineRect rect2;
      rect2.set(ZPoint(15, 2, 4), ZPoint(0, 1, 0), ZPoint(0, 0, 1), 30, 40);
      ASSERT_FALSE(zgeom::Intersects(rect, rect2));
    }

    {
      ZAffineRect rect2;
      rect2.set(ZPoint(1, 20, 4), ZPoint(0, 1, 0), ZPoint(0, 0, 1), 30, 40);
      ASSERT_TRUE(zgeom::Intersects(rect, rect2));
    }

    {
      ZAffineRect rect2;
      rect2.set(ZPoint(150, 200, 4), ZPoint(0, 1, 0), ZPoint(0, 0, 1), 30, 40);
      ASSERT_FALSE(zgeom::Intersects(rect, rect2));
    }

    ZCuboid box(1, 2, 3, 11, 22, 33);
    ASSERT_TRUE(zgeom::Intersects(rect, box));

    box.set(-10, -20, -30, 10, 20, 30);
    rect.setSize(4, 4);
    rect.setCenter(10, 0, 0);
    rect.setPlane(ZPoint(1, 0, 0), ZPoint(0, 1, 0));
    ASSERT_TRUE(zgeom::Intersects(rect, box));

    rect.setCenter(-10, -20, 0);
    ASSERT_TRUE(zgeom::Intersects(rect, box));

    rect.setCenter(-10, 20, 0);
    ASSERT_TRUE(zgeom::Intersects(rect, box));

    rect.setCenter(10, 20, 0);
    ASSERT_TRUE(zgeom::Intersects(rect, box));

    rect.setCenter(10, -20, 0);
    ASSERT_TRUE(zgeom::Intersects(rect, box));

    rect.setCenter(0, 0, 0);
    ASSERT_TRUE(zgeom::Intersects(rect, box));

    rect.setCenter(-15, -20, 0);
    ASSERT_FALSE(zgeom::Intersects(rect, box));

    rect.setCenter(-15, 20, 0);
    ASSERT_FALSE(zgeom::Intersects(rect, box));

    rect.setCenter(15, 20, 0);
    ASSERT_FALSE(zgeom::Intersects(rect, box));

    rect.setCenter(15, -20, 0);
    ASSERT_FALSE(zgeom::Intersects(rect, box));

    rect.setPlane(ZPoint(0, 1, 0), ZPoint(0, 0, 1));
    rect.setCenter(0, 20, 30);
    ASSERT_TRUE(zgeom::Intersects(rect, box));
    rect.setCenter(0, -20, 30);
    ASSERT_TRUE(zgeom::Intersects(rect, box));
    rect.setCenter(0, 20, -30);
    ASSERT_TRUE(zgeom::Intersects(rect, box));
    rect.setCenter(0, -20, -30);
    ASSERT_TRUE(zgeom::Intersects(rect, box));

  }
}

TEST(ZGeometry, Raster)
{
  {
    std::vector<ZIntPoint> nbrs;
    zgeom::raster::ForEachNeighbor(1, 2, 3, 1, 1, 1, [&](int x, int y, int z) {
      nbrs.push_back(ZIntPoint(x, y, z));
    });

    ASSERT_EQ(26, nbrs.size());
    ASSERT_EQ(ZIntPoint(0, 1, 2), nbrs[0]);
    ASSERT_EQ(ZIntPoint(2, 3, 4), nbrs[25]);
  }

  {
    std::vector<ZIntPoint> nbrs;
    zgeom::raster::ForEachNeighbor<1>(1, 2, 3, [&](int x, int y, int z) {
      nbrs.push_back(ZIntPoint(x, y, z));
    });

    ASSERT_EQ(6, nbrs.size());
    ASSERT_EQ(ZIntPoint(0, 2, 3), nbrs[0]);
    ASSERT_EQ(ZIntPoint(2, 2, 3), nbrs[1]);
  }

  {
    std::vector<ZIntPoint> nbrs;
    zgeom::raster::ForEachNeighbor<3>(0, 0, 0, [&](int x, int y, int z) {
      nbrs.push_back(ZIntPoint(x, y, z));
    });

    ASSERT_EQ(26, nbrs.size());
    ASSERT_EQ(ZIntPoint(-1, 0, 0), nbrs[0]);
    ASSERT_EQ(ZIntPoint(1, 0, 0), nbrs[1]);
    ASSERT_EQ(ZIntPoint(0, -1, 0), nbrs[2]);
    ASSERT_EQ(ZIntPoint(0, 1, 0), nbrs[3]);
    ASSERT_EQ(ZIntPoint(0, 0, -1), nbrs[4]);
    ASSERT_EQ(ZIntPoint(0, 0, 1), nbrs[5]);

    ASSERT_EQ(ZIntPoint(-1, -1, 0), nbrs[6]);
    ASSERT_EQ(ZIntPoint(1, -1, 0), nbrs[7]);
    ASSERT_EQ(ZIntPoint(-1, 1, 0), nbrs[8]);
    ASSERT_EQ(ZIntPoint(1, 1, 0), nbrs[9]);
    ASSERT_EQ(ZIntPoint(-1, 0, -1), nbrs[10]);
    ASSERT_EQ(ZIntPoint(1, 0, -1), nbrs[11]);
    ASSERT_EQ(ZIntPoint(-1, 0, 1), nbrs[12]);
    ASSERT_EQ(ZIntPoint(1, 0, 1), nbrs[13]);
    ASSERT_EQ(ZIntPoint(0, -1, -1), nbrs[14]);
    ASSERT_EQ(ZIntPoint(0, 1, -1), nbrs[15]);
    ASSERT_EQ(ZIntPoint(0, -1, 1), nbrs[16]);
    ASSERT_EQ(ZIntPoint(0, 1, 1), nbrs[17]);

    ASSERT_EQ(ZIntPoint(-1, -1, -1), nbrs[18]);
    ASSERT_EQ(ZIntPoint(1, -1, -1), nbrs[19]);
    ASSERT_EQ(ZIntPoint(- 1, 1, -1), nbrs[20]);
    ASSERT_EQ(ZIntPoint(1, 1, -1), nbrs[21]);
    ASSERT_EQ(ZIntPoint(-1, -1, 1), nbrs[22]);
    ASSERT_EQ(ZIntPoint(1, -1, 1), nbrs[23]);
    ASSERT_EQ(ZIntPoint(-1, 1, 1), nbrs[24]);
    ASSERT_EQ(ZIntPoint(1, 1, 1), nbrs[25]);

  }
}

TEST(ZGeometry, Int)
{
  std::pair<int, int> range = zgeom::ToIntRange(0, 1);
  ASSERT_EQ(0, range.first);
  ASSERT_EQ(0, range.second);

  range = zgeom::ToIntRange(0.5, 1.5);
  ASSERT_EQ(0, range.first);
  ASSERT_EQ(1, range.second);

  range = zgeom::ToIntRange(1, 1);
  ASSERT_EQ(1, range.first);
  ASSERT_EQ(1, range.second);

  range = zgeom::ToIntRange(-1.5, 1.5);
  ASSERT_EQ(-2, range.first);
  ASSERT_EQ(1, range.second);

}

#endif

#endif // ZGEOMETRYTEST_H
