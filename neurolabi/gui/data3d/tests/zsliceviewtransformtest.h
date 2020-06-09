#ifndef ZSLICEVIEWTRANSFORMTEST_H
#define ZSLICEVIEWTRANSFORMTEST_H

#include "data3d/zsliceviewtransform.h"
#include "geometry/zcuboid.h"

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

TEST(ZSliceViewTransform, Set)
{
  {
    ZSliceViewTransform t;
    t.setScale(2.0);
    t.setCutCenter(10, 20, 30);
    ASSERT_EQ(2.0, t.getScale());
    ASSERT_EQ(ZPoint(10, 20, 30), t.getCutCenter());

    t.incScale();
    ASSERT_DOUBLE_EQ(2.2, t.getScale());
    t.decScale();
    ASSERT_DOUBLE_EQ(2.0, t.getScale());

    t.setCutCenter(ZPoint(1, 2, 3), 10, 20);
    ASSERT_EQ(ZPoint(1, 2, 3), t.getCutCenter());
    ASSERT_TRUE(t.transform(ZPoint(1, 2, 3)).approxEquals(ZPoint(10, 20, 0)));

    ZPoint center = t.transform(
          ZPoint(5, 10, 0),
          neutu::data3d::ESpace::VIEW, neutu::data3d::ESpace::MODEL);
    t.zoomToViewRect(0, 0, 10, 20, 100, 200);

    ASSERT_EQ(10, t.getScale());
    ASSERT_EQ(neutu::geom2d::Point(10, 20), t.getAnchor());
    ASSERT_EQ(neutu::geom2d::Point(10, 20),
              t.getViewCanvasTransform().transform(neutu::geom2d::Point(0, 0)));
    ASSERT_EQ(neutu::geom2d::Point(-1, -2),
              t.getViewCanvasTransform().inverseTransform(neutu::geom2d::Point(0, 0)));
    ASSERT_EQ(center, t.inverseTransform(50, 100, 0));
  }

  {
    ZSliceViewTransform t;
    t.setCutCenter(ZPoint(1, 2, 3), 10, 20);

    ZPoint center = t.transform(
          ZPoint(5, 10, 0),
          neutu::data3d::ESpace::CANVAS, neutu::data3d::ESpace::MODEL);
    t.zoomToCanvasRect(0, 0, 10, 20, 100, 200);

    ASSERT_EQ(10, t.getScale());
    ASSERT_EQ(neutu::geom2d::Point(10, 20), t.getAnchor());
    ASSERT_EQ(neutu::geom2d::Point(10, 20),
              t.getViewCanvasTransform().transform(neutu::geom2d::Point(0, 0)));
    ASSERT_EQ(neutu::geom2d::Point(-1, -2),
              t.getViewCanvasTransform().inverseTransform(neutu::geom2d::Point(0, 0)));
    ASSERT_EQ(center, t.inverseTransform(50, 100, 0));
  }

  {
    ZSliceViewTransform t;
    t.setCutCenter(ZPoint(1, 2, 3), 10, 20);
    t.setCutDepth(ZPoint(0, 0, 0), 10);
    ASSERT_EQ(10, t.getCutDepth(ZPoint::ORIGIN));
  }

  {
    ZSliceViewTransform t;
    t.setCutCenter(ZPoint(1, 2, 3), 10, 20);

    t.translateModelViewTransform(100, 200, 300, 20, 30);

    ASSERT_EQ(ZPoint(100, 200, 300), t.inverseTransform(20, 30, 0));
  }

  {
    ZSliceViewTransform t;
    t.setCutCenter(ZPoint(1, 2, 3), 10, 20);

    ZPoint pt = t.inverseTransform(20, 30, 0);
    t.setScaleFixingCanvasMapped(2.0, 20, 30);
    ASSERT_EQ(2.0, t.getScale());
    ASSERT_EQ(pt, t.inverseTransform(20, 30, 0));

    t.setCutPlane(
          ZPoint(1, 2, 3), ZPoint(1, 1, 0).getNormalized(),
          ZPoint(1, -1, 0).getNormalized());
    pt = t.inverseTransform(3, 5, 0);
    t.setScaleFixingCanvasMapped(2.0, 3, 5);
    ASSERT_EQ(2.0, t.getScale());
    ASSERT_TRUE(pt.approxEquals(t.inverseTransform(3, 5, 0)));
  }

  {
    ZSliceViewTransform t;
    ZIntCuboid modelRange;
    modelRange.set(0, 0, 0, 10, 20, 30);
    t.fitModelRange(modelRange, 11, 21);
    ASSERT_TRUE(ZPoint(0, 0, 0).approxEquals(t.transform(0, 0, 0)));
  }

  {
    ZSliceViewTransform t;
    t.moveCutDepth(10);
    ASSERT_EQ(ZPoint(0, 0, 10), t.getCutCenter());

    ZIntCuboid modelRange;
    modelRange.setSize(100, 200, 300);
    modelRange.translate(10, 20, 30);

    t.fitModelRange(modelRange, 10, 20);
    ASSERT_DOUBLE_EQ(0.1, t.getScale());
    ASSERT_TRUE(ZPoint(-0.45, -0.45, 155).approxEquals(t.transform(10, 20, 165)))
        << t.transform(10, 20, 165) << t.transform(60, 120, 0);
  }

  {
    ZSliceViewTransform t;
    t.setScale(2.0);
    t.setCutCenter(1, 2, 3);
    t.setAnchor(5, 10);
    ASSERT_EQ(2.0, t.getScale());
    ASSERT_EQ(neutu::geom2d::Point(5, 10), t.getAnchor());
    ASSERT_EQ(ZPoint(1, 2, 3), t.getCutCenter());

    t.moveAnchorTo(10, 20);
    ASSERT_EQ(ZPoint(5, 10, 0), t.transform(ZPoint(1, 2, 3)));
  }

  {
    ZSliceViewTransform t;
    t.setScale(2.0);
    t.setCutPlane(neutu::EAxis::X, ZPoint(1, 2, 3));
    t.setAnchor(5, 10);
    ASSERT_EQ(2.0, t.getScale());
    ASSERT_EQ(neutu::geom2d::Point(5, 10), t.getAnchor());
    ASSERT_EQ(ZPoint(1, 2, 3), t.getCutCenter());

    t.moveAnchorTo(10, 20);
    ASSERT_EQ(ZPoint(5, 10, 0), t.transform(ZPoint(1, 2, 3)));
  }
}

TEST(ZSliceViewTransform, Transform)
{
  {
    ZSliceViewTransform t;
    t.setScale(2.0);
    t.setCutCenter(ZPoint(10, 20, 30), 100, 200);

    ZPoint pt = t.transform(10, 20, 30);
    ASSERT_EQ(ZPoint(100, 200, 0), pt);
    ASSERT_EQ(ZPoint(102, 204, 0), t.transform(11, 22, 30));

    ASSERT_EQ(ZPoint(10, 20, 30), t.inverseTransform(100, 200, 0));
    ASSERT_EQ(ZPoint(11, 22, 30), t.inverseTransform(102, 204, 0));
  }

  {
    ZSliceViewTransform t;
    t.setScale(2.0);
    t.setCutPlane(neutu::EAxis::X, ZPoint(10, 20, 30));
    t.setAnchor(100, 200);

    ZPoint pt = t.transform(10, 20, 30);
    ASSERT_EQ(ZPoint(100, 200, 0), pt);
    ASSERT_EQ(ZPoint(100, 204, 1), t.transform(11, 22, 30));

    ASSERT_EQ(ZPoint(10, 20, 30), t.inverseTransform(100, 200, 0));
    ASSERT_EQ(ZPoint(10, 22, 31), t.inverseTransform(102, 204, 0));
  }

  {
    ZSliceViewTransform t;
    t.setScale(2.0);
    t.setCutPlane(neutu::EAxis::Y, ZPoint(10, 20, 30));
    t.setAnchor(100, 200);

    ZPoint pt = t.transform(10, 20, 30);
    ASSERT_EQ(ZPoint(100, 200, 0), pt);

    ASSERT_EQ(ZPoint(10, 20, 30), t.inverseTransform(100, 200, 0));
  }

  {
    ZSliceViewTransform t;
    t.setScale(2.0);
    t.setCutPlane(
          ZPoint(10, 20, 30), ZPoint(1, 1, 0).getNormalized(),
          ZPoint(1, -1, 0).getNormalized());
    t.setAnchor(100, 200);

    ZPoint pt = t.transform(10, 20, 30);
    ASSERT_EQ(ZPoint(100, 200, 0), pt);

    ASSERT_EQ(ZPoint(10, 20, 30), t.inverseTransform(100, 200, 0));
  }

  {
    ZSliceViewTransform t;
    ZIntCuboid modelBox(-1, -2, -3, 1, 2, 3);
    ZCuboid box = t.getViewBox(modelBox);
    ASSERT_EQ(modelBox, box.toIntCuboid());

    t.setCutPlane(neutu::EAxis::X);
    box = t.getViewBox(modelBox);
    modelBox.shiftSliceAxis(neutu::EAxis::X);
    ASSERT_EQ(modelBox, box.toIntCuboid());
//    std::cout << box.toIntCuboid() << std::endl;
  }
}

#endif

#endif // ZSLICEVIEWTRANSFORMTEST_H
