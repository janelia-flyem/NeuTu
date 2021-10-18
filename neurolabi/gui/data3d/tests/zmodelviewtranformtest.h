#ifndef ZWORLDVIEWTRANFORMTEST_H
#define ZWORLDVIEWTRANFORMTEST_H

#include <cmath>

#include "data3d/zmodelviewtransform.h"

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

TEST(ZModelViewTransform, Basic)
{
  {
    ZModelViewTransform t;
    t.setCutPlane(neutu::EAxis::X, 5);
    ASSERT_EQ(neutu::EAxis::X, t.getSliceAxis());
//    ASSERT_DOUBLE_EQ(5.0, t.getCutDepth());
    ASSERT_TRUE(
          ZAffinePlane(ZPoint(5, 0, 0),ZPoint(0, 0, 1), ZPoint(0, 1, 0)).
          approxEquals(t.getCutPlane()));
  }

  {
    ZModelViewTransform t;
    t.setCutPlane(neutu::EAxis::Y, 3);
    ASSERT_EQ(neutu::EAxis::Y, t.getSliceAxis());
//    ASSERT_DOUBLE_EQ(3.0, t.getCutDepth());
    ASSERT_TRUE(
          ZAffinePlane(ZPoint(0, 3, 0),ZPoint(1, 0, 0), ZPoint(0, 0, 1)).
          approxEquals(t.getCutPlane())) << t.getCutPlane();
  }

  {
    ZModelViewTransform t;
    t.setCutPlane(neutu::EAxis::Z, 2);
    ASSERT_EQ(neutu::EAxis::Z, t.getSliceAxis());
//    ASSERT_DOUBLE_EQ(2.0, t.getCutDepth());
    ASSERT_TRUE(
          ZAffinePlane(ZPoint(0, 0, 2),ZPoint(1, 0, 0), ZPoint(0, 1, 0)).
          approxEquals(t.getCutPlane()));
  }

  {
    ZModelViewTransform t;
    t.setCutPlane(ZAffinePlane(ZPoint(5, 0, 0),ZPoint(0, 0, 1), ZPoint(0, 1, 0)));
    ASSERT_EQ(neutu::EAxis::ARB, t.getSliceAxis());
    ASSERT_TRUE(
          ZAffinePlane(ZPoint(5, 0, 0), ZPoint(0, 0, 1), ZPoint(0, 1, 0)).
          approxEquals(t.getCutPlane()));
    t.setCutPlane(neutu::EAxis::ARB, 1.0);
    ASSERT_TRUE(ZAffinePlane(ZPoint(4, 0, 0), ZPoint(0, 0, 1), ZPoint(0, 1, 0)).
                approxEquals(t.getCutPlane())) << t.getCutPlane();

    t.translateCutCenterOnPlane(10, 20);
    ASSERT_EQ(ZPoint(4, 20, 10), t.getCutCenter());

    t.moveCutDepth(5);
    ASSERT_EQ(ZPoint(-1, 20, 10), t.getCutCenter());
  }
}

TEST(ZModelViewTransform, transform)
{
  {
    ZModelViewTransform t;
    t.setCutPlane(neutu::EAxis::Z, ZPoint (1, 2, 3));
    ZPoint pt(1, 2, 3);
    ZPoint pt2 = t.transform(pt);
    ASSERT_EQ(ZPoint(0, 0, 0), pt2);
    ASSERT_EQ(pt, t.inverseTransform(pt2));

    t.setCutPlane(neutu::EAxis::X, ZPoint(1, 2, 3));
    pt2 = t.transform(pt);
    ASSERT_EQ(ZPoint(0, 0, 0), pt2);
    ASSERT_EQ(pt, t.inverseTransform(pt2));

    t.setCutPlane(neutu::EAxis::Y, ZPoint(1, 2, 3));
    pt2 = t.transform(pt);
    ASSERT_EQ(ZPoint(0, 0, 0), pt2);
    ASSERT_EQ(pt, t.inverseTransform(pt2));

    t.setCutPlane(neutu::EAxis::ARB, ZPoint(1, 2, 3));
    pt2 = t.transform(pt);
    ASSERT_EQ(ZPoint(0, 0, 0), pt2);
    ASSERT_EQ(pt, t.inverseTransform(pt2));

    pt.set(10, 20, 30);
    t.setCutPlane(neutu::EAxis::Z, ZPoint (1, 2, 3));
    pt2 = t.transform(pt);
    ASSERT_EQ(ZPoint(9, 18, 27), pt2);
    ASSERT_EQ(pt, t.inverseTransform(pt2));

    pt.set(10, 20, 30);
    t.setCutPlane(neutu::EAxis::Y, ZPoint (1, 2, 3));
    pt2 = t.transform(pt);
    ASSERT_EQ(ZPoint(9, 27, 18), pt2);
    ASSERT_EQ(pt, t.inverseTransform(pt2));

    pt.set(10, 20, 30);
    t.setCutPlane(neutu::EAxis::X, ZPoint (1, 2, 3));
    pt2 = t.transform(pt);
    ASSERT_EQ(ZPoint(27, 18, 9), pt2);
    ASSERT_EQ(pt, t.inverseTransform(pt2));

    t.setCutPlane(ZPoint(1, 2, 3), ZPoint(1, 1, 0).getNormalized(),
                  ZPoint(1, -1, 0).getNormalized());
    pt.set(std::sqrt(2) + 1, 2, 3);
    pt2 = t.transform(pt);
    ASSERT_TRUE(pt2.approxEquals(ZPoint(1.0, 1.0, 0.0))) << t << pt2;
  }

  {
    ZModelViewTransform t;
    t.setCutPlane(neutu::EAxis::Z, ZPoint (1, 2, 3));

    ZPoint boxSize(10, 20, 30);
    ZPoint ns = t.transformBoxSize(boxSize);
    ASSERT_EQ(ZPoint(10, 20, 30), ns);

    t.setCutPlane(neutu::EAxis::X, ZPoint (1, 2, 3));
    ns = t.transformBoxSize(boxSize);
    ASSERT_EQ(ZPoint(30, 20, 10), ns);

    t.setCutPlane(neutu::EAxis::Y, ZPoint (1, 2, 3));
    ns = t.transformBoxSize(boxSize);
    ASSERT_EQ(ZPoint(10, 30, 20), ns);

    t.setCutPlane(neutu::EAxis::ARB, ZPoint (1, 2, 3));
    ns = t.transformBoxSize(boxSize);
    ASSERT_EQ(ZPoint(boxSize.length(), boxSize.length(), boxSize.length()), ns);
  }

}

#endif

#endif // ZWORLDVIEWTRANFORMTEST_H
