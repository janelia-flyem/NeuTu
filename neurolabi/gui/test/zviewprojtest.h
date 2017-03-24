#ifndef ZVIEWPROJTEST_H
#define ZVIEWPROJTEST_H

#include "ztestheader.h"
#include "zviewproj.h"

#ifdef _USE_GTEST_

TEST(ZViewProj, basic)
{
  ZViewProj viewProj;

  ASSERT_FALSE(viewProj.isValid());
  ASSERT_TRUE(viewProj.getProjRegion().isNull());
  ASSERT_TRUE(viewProj.getViewPort().isNull());

  viewProj.setCanvasRect(QRect(0, 0, 100, 200));
  viewProj.update();

  ASSERT_FALSE(viewProj.isValid());
  ASSERT_TRUE(viewProj.getProjRegion().isNull());
  ASSERT_TRUE(viewProj.getViewPort().isNull());


  viewProj.setWidgetRect(QRect(0, 0, 100, 200));
  viewProj.setZoom(1);

  viewProj.update();

  ASSERT_TRUE(viewProj.isValid());
  ASSERT_TRUE(viewProj.getProjRegion().isValid());
  ASSERT_TRUE(viewProj.getViewPort().isValid());

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(100, viewProj.getViewPort().width());
  ASSERT_EQ(200, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRegion().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRegion().height());

  viewProj.setZoom(2);
  viewProj.update();

  ASSERT_TRUE(viewProj.isValid());
  ASSERT_TRUE(viewProj.getProjRegion().isValid());
  ASSERT_TRUE(viewProj.getViewPort().isValid());

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(50, viewProj.getViewPort().width());
  ASSERT_EQ(100, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRegion().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRegion().height());


  viewProj.setCanvasRect(QRect(0, 0, 100, 100));
  viewProj.update();

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(50, viewProj.getViewPort().width());
  ASSERT_EQ(100, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRegion().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRegion().height());

  viewProj.setCanvasRect(QRect(0, 0, 100, 300));
  viewProj.update();

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(50, viewProj.getViewPort().width());
  ASSERT_EQ(100, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRegion().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRegion().height());

  viewProj.setCanvasRect(QRect(0, 0, 200, 300));
  viewProj.update();

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(50, viewProj.getViewPort().width());
  ASSERT_EQ(100, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRegion().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRegion().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRegion().height());

  viewProj.setCanvasRect(QRect(30, 50, 200, 300));
  viewProj.setZoom(1);
  viewProj.update();

  ASSERT_EQ(30, viewProj.getViewPort().left());
  ASSERT_EQ(50, viewProj.getViewPort().top());
  ASSERT_EQ(70, viewProj.getViewPort().width());
  ASSERT_EQ(150, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(30, viewProj.getProjRegion().left());
  ASSERT_DOUBLE_EQ(50, viewProj.getProjRegion().top());
  ASSERT_DOUBLE_EQ(70, viewProj.getProjRegion().width());
  ASSERT_DOUBLE_EQ(150, viewProj.getProjRegion().height());

  viewProj.setZoom(2);
  viewProj.update();

  ASSERT_EQ(30, viewProj.getViewPort().left());
  ASSERT_EQ(50, viewProj.getViewPort().top());
  ASSERT_EQ(20, viewProj.getViewPort().width());
  ASSERT_EQ(50, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(60, viewProj.getProjRegion().left());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRegion().top());
  ASSERT_DOUBLE_EQ(40, viewProj.getProjRegion().width());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRegion().height());

}


#endif

#endif // ZVIEWPROJTEST_H
