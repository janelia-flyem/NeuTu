#ifndef ZAFFINERECTTEST_H
#define ZAFFINERECTTEST_H

#include "ztestheader.h"
#include "geometry/zgeometry.h"
#include "geometry/zaffinerect.h"
#include "geometry/zcuboid.h"

#ifdef _USE_GTEST_

TEST(ZAffineRect, Basic)
{
  {
    ZAffineRect rect;
    rect.set(ZPoint(1, 2, 3), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 512, 1024);

    ASSERT_EQ(512, rect.getWidth());
    ASSERT_EQ(1024, rect.getHeight());
    ASSERT_EQ(ZPoint(1, 0, 0), rect.getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rect.getV2());
    ASSERT_EQ(ZPoint(1, 2, 3), rect.getCenter());

    rect.setCenter(0, 0, 0);
    ASSERT_EQ(ZPoint(256, 512, 0), rect.getCorner(0));
    ASSERT_EQ(ZPoint(-256, 512, 0), rect.getCorner(1));
    ASSERT_EQ(ZPoint(-256, -512, 0), rect.getCorner(2));
    ASSERT_EQ(ZPoint(256, -512, 0), rect.getCorner(3));

    ASSERT_TRUE(
          ZLineSegment(256, 512, 0, -256, 512, 0).approxEquals(rect.getSide(0)));
    ASSERT_TRUE(
          ZLineSegment(-256, 512, 0, -256, -512, 0).approxEquals(rect.getSide(1)));
    ASSERT_TRUE(
          ZLineSegment(-256, -512, 0, 256, -512, 0).approxEquals(rect.getSide(2)));
    ASSERT_TRUE(
          ZLineSegment(256, -512, 0, 256, 512, 0).approxEquals(rect.getSide(3)));

    rect.setCenter(1, 2, 3);
    rect.translate(4, 5, 6);
    ASSERT_EQ(512, rect.getWidth());
    ASSERT_EQ(1024, rect.getHeight());
    ASSERT_EQ(ZPoint(1, 0, 0), rect.getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rect.getV2());
    ASSERT_EQ(ZPoint(5, 7, 9), rect.getCenter());

    rect.scale(10, 20);
    ASSERT_EQ(5120, rect.getWidth());
    ASSERT_EQ(20480, rect.getHeight());
    ASSERT_EQ(ZPoint(1, 0, 0), rect.getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rect.getV2());
    ASSERT_EQ(ZPoint(5, 7, 9), rect.getCenter());
  }
}

TEST(ZAffineRect, Partition)
{
  {
    ZAffineRect rect;
    rect.set(ZPoint(0, 0, 0), ZPoint(1, 0, 0), ZPoint(0, 1, 0), 512, 1024);
    std::vector<ZAffineRect> rectArray = zgeom::IntPartition(rect, 1, 1);

    ASSERT_EQ(1, (int) rectArray.size());
    ASSERT_EQ(512, rect.getWidth());
    ASSERT_EQ(ZPoint(0, 0, 0), rectArray[0].getCenter());
    ASSERT_EQ(ZPoint(1, 0, 0), rectArray[0].getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rectArray[0].getV2());
    ASSERT_EQ(512, rectArray[0].getWidth());
    ASSERT_EQ(1024, rectArray[0].getHeight());

    rectArray = zgeom::IntPartition(rect, 1, 2);
    ASSERT_EQ(2, (int) rectArray.size());
    ASSERT_EQ(ZPoint(-128, 0, 0), rectArray[0].getCenter());
    ASSERT_EQ(ZPoint(1, 0, 0), rectArray[0].getV1());
    ASSERT_EQ(ZPoint(0, 1, 0), rectArray[0].getV2());
    ASSERT_EQ(256, rectArray[0].getWidth());
    ASSERT_EQ(1024, rectArray[0].getHeight());

    rectArray = zgeom::IntPartition(rect, 2, 2);
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
    rectArray = zgeom::IntPartition(rect, 2, 2);
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
}

#endif

#endif // ZAFFINERECTTEST_H
