#ifndef ZPOINTTEST_H
#define ZPOINTTEST_H

#include <unordered_map>

#include "ztestheader.h"
#include "geometry/zintpoint.h"
#include "geometry/zpoint.h"

#ifdef _USE_GTEST_

TEST(ZIntPoint, Basic)
{
  ZIntPoint pt;
  ASSERT_TRUE(pt.isZero());

  pt.invalidate();
  ASSERT_FALSE(pt.isValid());

  pt.set(1, 2, 3);
  ASSERT_EQ(1, pt.getValue(neutu::EAxis::X));
  ASSERT_EQ(2, pt.getValue(neutu::EAxis::Y));
  ASSERT_EQ(3, pt.getValue(neutu::EAxis::Z));
  ASSERT_EQ(0, pt.getValue(neutu::EAxis::ARB));
}

TEST(ZIntPoint, ToPoint)
{
  ZIntPoint pt(1, 2, 3);
  ASSERT_EQ(ZPoint(1, 2, 3), pt.toPoint());
  ASSERT_EQ(ZPoint(0.5, 1.5, 2.5), pt.toMinCorner());
  ASSERT_EQ(ZPoint(1.5, 2.5, 3.5), pt.toMaxCorner());

  pt.set(-1, -2, -3);
  ASSERT_EQ(ZPoint(-1, -2, -3), pt.toPoint());
  ASSERT_EQ(ZPoint(-0.5, -1.5, -2.5), pt.toMaxCorner());
  ASSERT_EQ(ZPoint(-1.5, -2.5, -3.5), pt.toMinCorner());
}

TEST(ZPoint, Basic)
{
  ZPoint pt(0, 0, 0);
  ASSERT_TRUE(pt.isApproxOrigin());

  pt.setX(ZPoint::MIN_DIST * 0.5);
  ASSERT_TRUE(pt.isApproxOrigin());

  pt.normalize();
//  pt.print();
  ASSERT_FALSE(pt.isUnitVector());

  pt.setX(ZPoint::MIN_DIST * 2.0);
  ASSERT_FALSE(pt.isApproxOrigin());

  pt.normalize();
  ASSERT_TRUE(pt.isUnitVector());
}

TEST(ZPoint, Relation)
{
  ZPoint pt1(1, 0, 0);
  ZPoint pt2(0, 1, 0);
  ASSERT_TRUE(pt1.isPendicularTo(pt2));
  ASSERT_TRUE(pt2.isPendicularTo(pt1));

  pt1.set(0, 0, 0);
  ASSERT_FALSE(pt1.isPendicularTo(pt2));
  ASSERT_FALSE(pt2.isPendicularTo(pt1));

  pt1.set(0.1, 0, 0);
  ASSERT_TRUE(pt1.isPendicularTo(pt2));
  ASSERT_TRUE(pt2.isPendicularTo(pt1));

  pt1.set(0.1, 0.1, 0);
  ASSERT_FALSE(pt1.isPendicularTo(pt2));
  ASSERT_FALSE(pt2.isPendicularTo(pt1));

  pt1.set(1, 1, 0);
  pt2.set(2, 2, 0);
  ASSERT_TRUE(pt1.isParallelTo(pt2));

  pt2.set(2.1, 2, 0);
  ASSERT_FALSE(pt1.isParallelTo(pt2));

  pt2.set(0, 0, 0);
  ASSERT_FALSE(pt1.isParallelTo(pt2));
}

TEST(ZPoint, toIntPoint)
{
  ZPoint pt(1.0, 2.6, 4.1);
  ASSERT_EQ(ZIntPoint(1, 3, 4), pt.toIntPoint());

  pt.set(1.5, 2.5, 3.5);
  ASSERT_EQ(ZIntPoint(2, 3, 4), pt.toIntPoint());

  pt.set(-1.5, -2.5, -3.5);
  ASSERT_EQ(ZIntPoint(-1, -2, -3), pt.toIntPoint());

  ASSERT_FALSE(pt.hasIntCoord());
  ASSERT_TRUE(pt.toIntPoint().toPoint().hasIntCoord());
}

TEST(ZIntPoint, Operator)
{
  ZIntPoint pt1(0, 1, 2);
  pt1 = pt1 + 1;
  ASSERT_EQ(pt1, ZIntPoint(1, 2, 3));

  pt1.invalidate();
  ASSERT_FALSE((-pt1).isValid());

  ASSERT_FALSE((pt1 + 1).isValid());

  pt1 += ZIntPoint(0, 1, 2);
  ASSERT_FALSE(pt1.isValid());

  pt1.set(0, 1, 2);
  ASSERT_TRUE(pt1.isValid());

  ASSERT_FALSE((pt1 + INT_MIN).isValid());

  pt1 += ZIntPoint(10, 20, 30);
  ASSERT_EQ(pt1, ZIntPoint(10, 21, 32));


  pt1 = ZIntPoint(1, 2, 3) + ZIntPoint(10, 20, 30);
  ASSERT_EQ(pt1, ZIntPoint(11, 22, 33));


  pt1.set(0, 1, 2);
  pt1 = pt1 - 1;
  ASSERT_EQ(pt1, ZIntPoint(-1, 0, 1));
  ASSERT_EQ(-pt1, ZIntPoint(1, 0, -1));

  pt1.invalidate();
  ASSERT_FALSE((pt1 - 1).isValid());

  pt1 -= ZIntPoint(0, 1, 2);
  ASSERT_FALSE(pt1.isValid());

  pt1.set(0, 1, 2);
  ASSERT_TRUE(pt1.isValid());

  ASSERT_FALSE((pt1 - INT_MIN).isValid());

  pt1 -= ZIntPoint(10, 20, 30);
  ASSERT_EQ(pt1, ZIntPoint(-10, -19, -28));

  pt1 = ZIntPoint(1, 2, 3) - ZIntPoint(10, 20, 30);
  ASSERT_EQ(pt1, ZIntPoint(-9, -18, -27));

  pt1.set(0, 1, 2);
  pt1 = pt1 * 2;
  ASSERT_EQ(pt1, ZIntPoint(0, 2, 4));

  pt1.invalidate();
  ASSERT_FALSE((pt1 * 2).isValid());

  pt1 *= ZIntPoint(1, 2, 3);
  ASSERT_FALSE(pt1.isValid());

  pt1.set(0, 1, 2);
  ASSERT_TRUE(pt1.isValid());

  ASSERT_FALSE((pt1 * INT_MIN).isValid());

  pt1 *= ZIntPoint(10, 20, 30);
  ASSERT_EQ(pt1, ZIntPoint(0, 20, 60));


  pt1 = ZIntPoint(1, 2, 3) * ZIntPoint(10, 20, 30);
  ASSERT_EQ(pt1, ZIntPoint(10, 40, 90));


  pt1.set(0, 1, 2);
  pt1 = pt1 / 2;
  ASSERT_EQ(pt1, ZIntPoint(0, 0, 1));

  pt1.invalidate();
  ASSERT_FALSE((pt1 / 2).isValid());

  pt1 /= ZIntPoint(1, 2, 3);
  ASSERT_FALSE(pt1.isValid());

  pt1.set(0, 1, 2);
  ASSERT_TRUE(pt1.isValid());

  ASSERT_FALSE((pt1 / INT_MIN).isValid());

  pt1 /= ZIntPoint(10, 20, 30);
  ASSERT_EQ(pt1, ZIntPoint(0, 0, 0));


  pt1 =  ZIntPoint(10, 20, 30) / ZIntPoint(1, 2, 3);
  ASSERT_EQ(pt1, ZIntPoint(10, 10, 10));

  ASSERT_TRUE(ZIntPoint(1, 1, 1).definiteLessThan(ZIntPoint(2, 2, 2)));
  ASSERT_TRUE(ZIntPoint(1, 1, 1).definiteLessThan(ZIntPoint(1, 2, 1)));
  ASSERT_TRUE(ZIntPoint(1, 1, 1).definiteLessThan(ZIntPoint(1, 1, 2)));
  ASSERT_FALSE(ZIntPoint(1, 1, 1).definiteLessThan(ZIntPoint(1, 1, 1)));
  ASSERT_FALSE(ZIntPoint(1, 1, 1).definiteLessThan(ZIntPoint(0, 1, 1)));
  ASSERT_FALSE(ZIntPoint(1, 1, 1).definiteLessThan(ZIntPoint(1, 2, 0)));
  ASSERT_FALSE(ZIntPoint(1, 1, 1).definiteLessThan(ZIntPoint(0, 1, 2)));
  ASSERT_FALSE(ZIntPoint(1, 1, 1).definiteLessThan(ZIntPoint(0, 2, 2)));
}

TEST(ZIntPoint, Hash)
{
  std::unordered_map<ZIntPoint, int> pointMap;
  pointMap[ZIntPoint(1, 2, 3)]  = 1;
  pointMap[ZIntPoint(4, 5, 6)]  = 2;

  ASSERT_EQ(1, pointMap.at(ZIntPoint(1, 2, 3)));
  ASSERT_EQ(2, pointMap.at(ZIntPoint(4, 5, 6)));

  pointMap[ZIntPoint(1, 2, 3)]  = 3;
  ASSERT_EQ(3, pointMap.at(ZIntPoint(1, 2, 3)));
}

#endif


#endif // ZPOINTTEST_H
