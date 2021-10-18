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

    ASSERT_EQ(ZPoint(256, 512, 0), rect.getMaxCorner());
    ASSERT_EQ(ZPoint(-256, -512, 0), rect.getMinCorner());

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

    rect.setCenter(0, 0, 0);
    rect.setSize(10, -10);
    ASSERT_TRUE(rect.isEmpty());
    ASSERT_EQ(10, rect.getWidth());
    ASSERT_EQ(0, rect.getHeight());
    ASSERT_TRUE(rect.contains(ZPoint(0, 0, 0)));
    ASSERT_TRUE(rect.contains(ZPoint(1, 0, 0)));
    ASSERT_FALSE(rect.contains(ZPoint(0, 1, 0)));

    rect.setSize(-10, 10);
    ASSERT_TRUE(rect.isEmpty());
    ASSERT_EQ(0, rect.getWidth());
    ASSERT_EQ(10, rect.getHeight());
    ASSERT_TRUE(rect.contains(ZPoint(0, 0, 0)));
    ASSERT_TRUE(rect.contains(ZPoint(0, 1, 0)));
    ASSERT_FALSE(rect.contains(ZPoint(1, 0, 0)));

    ASSERT_EQ(ZPoint(0, 5, 0), rect.getCorner(0));
    ASSERT_EQ(ZPoint(0, 5, 0), rect.getCorner(1));
    ASSERT_EQ(ZPoint(0, -5, 0), rect.getCorner(2));
    ASSERT_EQ(ZPoint(0, -5, 0), rect.getCorner(3));
  }
}

TEST(ZAffineRect, setSize)
{
  ZAffineRect rect = ZAffineRectBuilder()
      .at(ZPoint(1, 2, 3))
      .on(ZPoint(1, 0, 0), ZPoint(0, 1, 0))
      .withSize(100, 200);

  for (int index = 0; index < 4; ++index) {
    ZAffineRect rect2 = rect;
    rect2.setSizeWithCornerFixed(20, 30, index);
    ASSERT_EQ(20, rect2.getWidth());
    ASSERT_EQ(30, rect2.getHeight());
    ASSERT_EQ(rect.getCorner(index), rect2.getCorner(index));
  }

  {
    ZAffineRect rect2 = rect;
    rect2.setSizeWithMinCornerFixed(20, 30);
    ASSERT_EQ(20, rect2.getWidth());
    ASSERT_EQ(30, rect2.getHeight());
    ASSERT_EQ(rect.getMinCorner(), rect2.getMinCorner());
  }

  {
    ZAffineRect rect2 = rect;
    rect2.setSizeWithMaxCornerFixed(20, 30);
    ASSERT_EQ(20, rect2.getWidth());
    ASSERT_EQ(30, rect2.getHeight());
    ASSERT_EQ(rect.getMaxCorner(), rect2.getMaxCorner());
  }

  // Test corner cases
  {
    ZAffineRect rect2 = rect;
    rect2.setSizeWithCornerFixed(20, 30, -1);
    ASSERT_EQ(20, rect2.getWidth());
    ASSERT_EQ(30, rect2.getHeight());
    ASSERT_EQ(rect.getCenter(), rect2.getCenter());
  }

  {
    ZAffineRect rect2 = rect;
    rect2.setSizeWithCornerFixed(20, 30, 4);
    ASSERT_EQ(20, rect2.getWidth());
    ASSERT_EQ(30, rect2.getHeight());
    ASSERT_EQ(rect.getCenter(), rect2.getCenter());
  }

  {
    for (int index = 0; index < 4; ++index) {
      ZAffineRect rect2 = rect;
      rect2.setSizeWithCornerFixed(-20, -30, index);
      ASSERT_EQ(0, rect2.getWidth());
      ASSERT_EQ(0, rect2.getHeight());
      ASSERT_EQ(rect.getCorner(index), rect2.getCorner(index));
    }
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
