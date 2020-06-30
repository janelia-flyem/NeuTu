#ifndef ZSTACKVIEWPARAMTEST_H
#define ZSTACKVIEWPARAMTEST_H

#include "ztestheader.h"
#include "zstackviewparam.h"
#include "geometry/zaffinerect.h"
#include "zarbsliceviewparam.h"

#ifdef _USE_GTEST_
TEST(ZStackViewParam, Basic)
{
  ZStackViewParam param;
  ASSERT_EQ(neutu::EAxis::Z, param.getSliceAxis());
  ASSERT_FALSE(param.isValid());

  param.setSize(128, 128, neutu::data3d::ESpace::MODEL);
  ASSERT_TRUE(param.isValid());

  param.closeViewPort();
  ASSERT_FALSE(param.isValid());

  param.openViewPort();
  ASSERT_TRUE(param.isValid());

  ZStackViewParam param2 = param;
//  param2.setViewPort(QRect(15, 25, 20, 30));
  ASSERT_TRUE(param.contains(param2));

  param.closeViewPort();
  ASSERT_FALSE(param.contains(param2));

  param.openViewPort();
  param2.moveCutDepth(1);
  ASSERT_FALSE(param.contains(param2));
//  ASSERT_TRUE(param.containsViewport(param2));


  param2.setCutDepth(ZPoint::ORIGIN, param.getCutDepth(ZPoint::ORIGIN));
  param.setSliceAxis(neutu::EAxis::X);
  ASSERT_FALSE(param.contains(param2));
  param2.setSliceAxis(param.getSliceAxis());
  ASSERT_TRUE(param.contains(param2));

//  param2.setViewPort(QRect(15, 25, 200, 300));
  param2.setSize(256, 256, neutu::data3d::ESpace::MODEL);
  ASSERT_FALSE(param.contains(param2));
  param2.closeViewPort();
  ASSERT_TRUE(param.contains(param2));

  param.closeViewPort();
  ASSERT_TRUE(param.contains(param2));
}

TEST(ZStackViewParam, Size)
{
  ZStackViewParam param;
  ZSliceViewTransform t;
  t.setCutCenter(63.5, 63.5, 0);
  param.set(t, 128, 128, neutu::data3d::ESpace::CANVAS);

  ASSERT_EQ(128, param.getWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128*128, param.getArea(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getWidth(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(128, param.getHeight(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(128*128, param.getArea(neutu::data3d::ESpace::MODEL));

  ASSERT_EQ(128, param.getIntWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getIntHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getIntWidth(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(128, param.getIntHeight(neutu::data3d::ESpace::MODEL));


  t.setScale(2.0);
  param.setTransform(t);
  ASSERT_EQ(2.0, param.getZoomRatio());
  ASSERT_EQ(0, param.getZoomLevel());

  ASSERT_EQ(128, param.getWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128*128, param.getArea(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(64, param.getWidth(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(64, param.getHeight(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(64*64, param.getArea(neutu::data3d::ESpace::MODEL));

  ASSERT_EQ(128, param.getIntWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getIntHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(64, param.getIntWidth(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(64, param.getIntHeight(neutu::data3d::ESpace::MODEL));

  param.setSize(100, 300, neutu::data3d::ESpace::MODEL);
  ASSERT_EQ(200, param.getWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(100, param.getWidth(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(100, param.getWidth(neutu::data3d::ESpace::VIEW));

  ASSERT_EQ(600, param.getHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(300, param.getHeight(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(300, param.getHeight(neutu::data3d::ESpace::VIEW));

  ASSERT_EQ(100*300, param.getArea(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(200*600, param.getArea(neutu::data3d::ESpace::CANVAS));

  param.setSize(100, 300, neutu::data3d::ESpace::VIEW);
  ASSERT_EQ(200, param.getWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(100, param.getWidth(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(100, param.getWidth(neutu::data3d::ESpace::VIEW));

  ASSERT_EQ(600, param.getHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(300, param.getHeight(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(300, param.getHeight(neutu::data3d::ESpace::VIEW));

  ASSERT_EQ(100*300, param.getArea(neutu::data3d::ESpace::MODEL));
  ASSERT_EQ(200*600, param.getArea(neutu::data3d::ESpace::CANVAS));
}

TEST(ZStackViewParam, Discrete)
{
  ZStackViewParam param;
  ZSliceViewTransform t;
  t.setCutCenter(63.5, 63.5, 0);
  param.set(t, 128, 128, neutu::data3d::ESpace::MODEL);

  ASSERT_TRUE(param.getIntCutRect().contains(param.getCutRect()))
      << param.getIntCutRect() << std::endl << param.getCutRect();
  ASSERT_FALSE(param.getCutRect().contains(param.getIntCutRect()))
      << param.getIntCutRect() << std::endl << param.getCutRect();;

  t.setScale(2.0);
  param.setTransform(t);
  ASSERT_TRUE(param.getIntCutRect().contains(param.getCutRect()))
      << param.getIntCutRect() << std::endl << param.getCutRect();
  ASSERT_FALSE(param.getCutRect().contains(param.getIntCutRect()));

  t.setScale(0.5);
  param.setTransform(t);
  ASSERT_TRUE(param.getIntCutRect().contains(param.getCutRect()))
      << param.getIntCutRect() << std::endl << param.getCutRect();
  ASSERT_FALSE(param.getCutRect().contains(param.getIntCutRect()));

  ZArbSliceViewParam arbParam = param.toArbSliceViewParam();
  ASSERT_EQ(param.getIntCutRect(), arbParam.getAffineRect());
  ASSERT_NE(param.getCutRect(), arbParam.getAffineRect());

  t.setCutPlane(
        t.getCutCenter(), ZPoint(1, 1, 0).getNormalized(),
        ZPoint(1, -1, 0).getNormalized());
  ASSERT_TRUE(param.getIntCutRect().contains(param.getCutRect()))
      << param.getIntCutRect() << std::endl << param.getCutRect();
  ASSERT_FALSE(param.getCutRect().contains(param.getIntCutRect()));
}

TEST(ZStackViewParam, Contains)
{
  ZStackViewParam param;
  ZSliceViewTransform t;
  t.setCutCenter(63.5, 63.5, 0);
  param.set(t, 128, 128, neutu::data3d::ESpace::MODEL);

  ASSERT_TRUE(param.contains(param.getCutCenter()));
  ASSERT_TRUE(param.contains(ZPoint(63.5, 63.5, -0.5)));
  ASSERT_FALSE(param.contains(ZPoint(63.5, 63.5, 0.5)));
  ASSERT_TRUE(param.contains(ZPoint(128, 128, -0.5)));
  ASSERT_TRUE(param.contains(ZPoint(191.5, 191.5, -0.5)));
  ASSERT_FALSE(param.contains(ZPoint(192, 191.5, -0.5)));

  t.setAnchor(64, 64);
  param.setTransform(t);
  ASSERT_TRUE(param.contains(param.getCutCenter()));
  ASSERT_TRUE(param.contains(ZPoint(63.5, 63.5, -0.5)));
  ASSERT_FALSE(param.contains(ZPoint(63.5, 63.5, 0.5)));
  ASSERT_FALSE(param.contains(ZPoint(191.5, 191.5, -0.5)));
}

TEST(ZStackViewParam, CutRect)
{
  ZStackViewParam param;
  ZSliceViewTransform t;
  t.setCutCenter(1, 2, 3);
  param.set(t, 10, 20, neutu::data3d::ESpace::MODEL);

  ZAffineRect rect = param.getCutRect();
  ASSERT_EQ(ZPoint(6, 12, 3), rect.getCenter());
  ASSERT_EQ(10, rect.getWidth());
  ASSERT_EQ(20, rect.getHeight());

  t.setAnchor(5, 10);
  param.set(t, 10, 20, neutu::data3d::ESpace::MODEL);
  rect = param.getCutRect();
//  std::cout << rect << std::endl;
  ASSERT_EQ(ZPoint(1, 2, 3), rect.getCenter());
  ASSERT_EQ(10, rect.getWidth());
  ASSERT_EQ(20, rect.getHeight());

  rect = param.getIntCutRect();
  ASSERT_EQ(ZPoint(1, 2, 3), rect.getCenter());
  ASSERT_EQ(10, rect.getWidth());
  ASSERT_EQ(20, rect.getHeight());

  rect = param.getIntCutRect(ZIntCuboid(0, 1, 2, 3, 4, 5));
  ASSERT_EQ(ZPoint(2, 3, 3), rect.getCenter());
  ASSERT_EQ(4, rect.getWidth());
  ASSERT_EQ(4, rect.getHeight());

  rect = param.getIntCutRect(ZIntCuboid(0, 1, 2, 4, 7, 5));
  ASSERT_EQ(ZPoint(2, 4, 3), rect.getCenter());
  ASSERT_EQ(5, rect.getWidth());
  ASSERT_EQ(7, rect.getHeight());
//  std::cout << rect << std::endl;
}

TEST(ZStackViewParam, Viewport)
{
  ZStackViewParam param;
  param.setViewport(128, 128, neutu::data3d::ESpace::CANVAS);
  param.setCutCenter(ZIntPoint(1, 2, 3));
  param.openViewPort();
  ASSERT_FALSE(param.isViewportEmpty());
  ASSERT_TRUE(param.isValid());
  ASSERT_FALSE(param.getCutRect().isEmpty());
  ASSERT_EQ(ZPoint(65, 66, 3), param.getCutRect().getCenter());

  param.closeViewPort();
  ASSERT_TRUE(param.isViewportEmpty());
  ASSERT_FALSE(param.isValid());
  ASSERT_EQ(0, param.getWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(0, param.getHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(0, param.getIntWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(0, param.getIntHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(0, param.getArea(neutu::data3d::ESpace::CANVAS));
  ASSERT_TRUE(param.getCutRect().isEmpty());
  ASSERT_EQ(ZPoint(1, 2, 3), param.getCutRect().getCenter());
  ASSERT_TRUE(param.getIntCutRect().isEmpty());
  ASSERT_EQ(ZPoint(1, 2, 3), param.getIntCutRect().getCenter());

  param.openViewPort();
  ASSERT_FALSE(param.isViewportEmpty());
  ASSERT_TRUE(param.isValid());
  ASSERT_EQ(128, param.getWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getIntWidth(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128, param.getIntHeight(neutu::data3d::ESpace::CANVAS));
  ASSERT_EQ(128*128, param.getArea(neutu::data3d::ESpace::CANVAS));
  ASSERT_FALSE(param.getCutRect().isEmpty());
  ASSERT_EQ(ZPoint(65, 66, 3), param.getCutRect().getCenter());
}

#endif

#endif // ZSTACKVIEWPARAMTEST_H
