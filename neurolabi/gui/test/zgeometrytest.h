#ifndef ZGEOMETRYTEST_H
#define ZGEOMETRYTEST_H

#include "ztestheader.h"
#include "geometry/zaffinerect.h"
#include "geometry/zgeometry.h"
#include "geometry/zplane.h"
#include "geometry/zaffineplane.h"
#include "geometry/zlinesegment.h"
#include "geometry/zcuboid.h"

#ifdef _USE_GTEST_
TEST(ZGeometry, Util)
{
  ASSERT_EQ(0, zgeom::GetZoomLevel(1));
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

    ZPoint pt = ap2.align(ZPoint(2, 4, 6));
    ASSERT_DOUBLE_EQ(2.0, pt.getX());
    ASSERT_DOUBLE_EQ(3.0, pt.getY());
    ASSERT_DOUBLE_EQ(1.0, pt.getZ());

    ap2.setOffset(ZPoint(2, 0, 0));
    ASSERT_FALSE(ap.onSamePlane(ap2));
  }
}

TEST(ZGeometry, ZAffineRect)
{
  {
    ZAffineRect rect;
    rect.set(ZPoint(0, 0, 0), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 512, 1024);

    ASSERT_EQ(512, rect.getWidth());
    ASSERT_EQ(ZPoint(1, 0, 0), rect.getV1());

    std::vector<ZAffineRect> rectArray = zgeom::Partition(rect, 1, 1);

    ASSERT_EQ(1, (int) rectArray.size());
    ASSERT_EQ(512, rect.getWidth());
    ASSERT_EQ(ZPoint(0, 0, 0), rectArray[0].getCenter());
    ASSERT_EQ(ZPoint(1, 0, 0), rectArray[0].getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rectArray[0].getV2());
    ASSERT_EQ(512, rectArray[0].getWidth());
    ASSERT_EQ(1024, rectArray[0].getHeight());

    rectArray = zgeom::Partition(rect, 1, 2);
    ASSERT_EQ(2, (int) rectArray.size());
    ASSERT_EQ(ZPoint(-128, 0, 0), rectArray[0].getCenter());
    ASSERT_EQ(ZPoint(1, 0, 0), rectArray[0].getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rectArray[0].getV2());
    ASSERT_EQ(256, rectArray[0].getWidth());
    ASSERT_EQ(1024, rectArray[0].getHeight());

    rectArray = zgeom::Partition(rect, 2, 2);
    ASSERT_EQ(4, (int) rectArray.size());
    ASSERT_EQ(ZPoint(-128, -256, 0), rectArray[0].getCenter());
    ASSERT_EQ(ZPoint(1, 0, 0), rectArray[0].getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rectArray[0].getV2());
    ASSERT_EQ(256, rectArray[0].getWidth());
    ASSERT_EQ(512, rectArray[0].getHeight());

    ASSERT_EQ(ZPoint(128, 256, 0), rectArray[3].getCenter());
    ASSERT_EQ(ZPoint(1, 0, 0), rectArray[3].getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rectArray[3].getV2());
    ASSERT_EQ(256, rectArray[3].getWidth());
    ASSERT_EQ(512, rectArray[3].getHeight());

    rect.set(ZPoint(0, 0, 0), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 511, 1025);
    rectArray = zgeom::Partition(rect, 2, 2);
    ASSERT_EQ(4, (int) rectArray.size());
    ASSERT_EQ(ZPoint(-127, -256, 0), rectArray[0].getCenter());
    ASSERT_EQ(ZPoint(1, 0, 0), rectArray[0].getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rectArray[0].getV2());
    ASSERT_EQ(256, rectArray[0].getWidth());
    ASSERT_EQ(513, rectArray[0].getHeight());

    ASSERT_EQ(ZPoint(128, 257, 0), rectArray[3].getCenter());
    ASSERT_EQ(ZPoint(1, 0, 0), rectArray[3].getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rectArray[3].getV2());
    ASSERT_EQ(255, rectArray[3].getWidth());
    ASSERT_EQ(512, rectArray[3].getHeight());
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
    ZCuboid box(1, 2, 3, 11, 22, 33);

    ZAffineRect rect;
    rect.set(ZPoint(1, 2, 4), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 10, 20);

    ASSERT_TRUE(zgeom::Intersects(rect, box));

//    std::cout << zgeom::Intersects(rect, box) << std::endl;
  }
}

#endif

#endif // ZGEOMETRYTEST_H
