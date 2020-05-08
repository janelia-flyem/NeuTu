#ifndef ZWORLDVIEWTRANFORMTEST_H
#define ZWORLDVIEWTRANFORMTEST_H

#include "ztestheader.h"
#include "data3d/zworldviewtransform.h"

#ifdef _USE_GTEST_

TEST(ZWorldViewTransform, Basic)
{
  ZWorldViewTransform t;

  t.setCutPlane(neutu::EAxis::X, 5);
  ASSERT_EQ(neutu::EAxis::X, t.getSliceAxis());
  ASSERT_DOUBLE_EQ(5.0, t.getCutDepth());
  ASSERT_TRUE(
        ZAffinePlane(ZPoint(5, 0, 0),ZPoint(0, 0, 1), ZPoint(0, 1, 0)).
        approxEquals(t.getCutPlane()));

  t.setCutPlane(neutu::EAxis::Y, 3);
  ASSERT_EQ(neutu::EAxis::Y, t.getSliceAxis());
  ASSERT_DOUBLE_EQ(3.0, t.getCutDepth());
  ASSERT_TRUE(
        ZAffinePlane(ZPoint(0, 3, 0),ZPoint(1, 0, 0), ZPoint(0, 0, 1)).
        approxEquals(t.getCutPlane()));

  t.setCutPlane(neutu::EAxis::Z, 2);
  ASSERT_EQ(neutu::EAxis::Z, t.getSliceAxis());
  ASSERT_DOUBLE_EQ(2.0, t.getCutDepth());
  ASSERT_TRUE(
        ZAffinePlane(ZPoint(0, 0, 2),ZPoint(1, 0, 0), ZPoint(0, 1, 0)).
        approxEquals(t.getCutPlane()));

  t.setCutPlane(ZAffinePlane(ZPoint(5, 0, 0),ZPoint(0, 0, 1), ZPoint(0, 1, 0)));
  ASSERT_EQ(neutu::EAxis::ARB, t.getSliceAxis());
  ASSERT_TRUE(
        ZAffinePlane(ZPoint(5, 0, 0), ZPoint(0, 0, 1), ZPoint(0, 1, 0)).
        approxEquals(t.getCutPlane()));
  t.setCutPlane(neutu::EAxis::ARB, 1.0);
  ASSERT_TRUE(ZAffinePlane(ZPoint(4, 0, 0), ZPoint(0, 0, 1), ZPoint(0, 1, 0)).
              approxEquals(t.getCutPlane())) << t.getCutPlane();
}

#endif

#endif // ZWORLDVIEWTRANFORMTEST_H
