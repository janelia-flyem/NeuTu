#ifndef ZVIEWPROJTEST_H
#define ZVIEWPROJTEST_H

#include "ztestheader.h"
#include "zviewproj.h"

#ifdef _USE_GTEST_

TEST(ZViewProj, basic)
{
  ZViewProj viewProj;

  ASSERT_FALSE(viewProj.isValid());
  ASSERT_TRUE(viewProj.getProjRect().isNull());
  ASSERT_TRUE(viewProj.getViewPort().isNull());

  viewProj.setCanvasRect(QRect(0, 0, 100, 200));
//  viewProj.update();

  ASSERT_FALSE(viewProj.isValid());
  ASSERT_TRUE(viewProj.getProjRect().isNull());
  ASSERT_TRUE(viewProj.getViewPort().isNull());


  viewProj.setWidgetRect(QRect(0, 0, 100, 200));
  viewProj.setZoom(1);

//  viewProj.update();

  ASSERT_TRUE(viewProj.isValid());
  ASSERT_TRUE(viewProj.getProjRect().isValid());
  ASSERT_TRUE(viewProj.getViewPort().isValid());

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(100, viewProj.getViewPort().width());
  ASSERT_EQ(200, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRect().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRect().height());

  viewProj.setZoom(2);
//  viewProj.update();

  ASSERT_TRUE(viewProj.isValid());
  ASSERT_TRUE(viewProj.getProjRect().isValid());
  ASSERT_TRUE(viewProj.getViewPort().isValid());

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(50, viewProj.getViewPort().width());
  ASSERT_EQ(100, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRect().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRect().height());


  viewProj.setCanvasRect(QRect(0, 0, 100, 100));
//  viewProj.update();

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(50, viewProj.getViewPort().width());
  ASSERT_EQ(100, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRect().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRect().height());

  viewProj.setCanvasRect(QRect(0, 0, 100, 300));
//  viewProj.update();

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(50, viewProj.getViewPort().width());
  ASSERT_EQ(100, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRect().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRect().height());

  viewProj.setCanvasRect(QRect(0, 0, 200, 300));
//  viewProj.update();

  ASSERT_EQ(0, viewProj.getViewPort().left());
  ASSERT_EQ(0, viewProj.getViewPort().top());
  ASSERT_EQ(50, viewProj.getViewPort().width());
  ASSERT_EQ(100, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().left());
  ASSERT_DOUBLE_EQ(0, viewProj.getProjRect().top());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRect().width());
  ASSERT_DOUBLE_EQ(200, viewProj.getProjRect().height());

  viewProj.setCanvasRect(QRect(30, 50, 200, 300));
  viewProj.setZoom(1);
//  viewProj.update();

  ASSERT_EQ(30, viewProj.getViewPort().left());
  ASSERT_EQ(50, viewProj.getViewPort().top());
  ASSERT_EQ(70, viewProj.getViewPort().width());
  ASSERT_EQ(150, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(30, viewProj.getProjRect().left());
  ASSERT_DOUBLE_EQ(50, viewProj.getProjRect().top());
  ASSERT_DOUBLE_EQ(70, viewProj.getProjRect().width());
  ASSERT_DOUBLE_EQ(150, viewProj.getProjRect().height());

  viewProj.setZoom(2);
//  viewProj.update();

  ASSERT_EQ(30, viewProj.getViewPort().left());
  ASSERT_EQ(50, viewProj.getViewPort().top());
  ASSERT_EQ(20, viewProj.getViewPort().width());
  ASSERT_EQ(50, viewProj.getViewPort().height());


  ASSERT_DOUBLE_EQ(60, viewProj.getProjRect().left());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRect().top());
  ASSERT_DOUBLE_EQ(40, viewProj.getProjRect().width());
  ASSERT_DOUBLE_EQ(100, viewProj.getProjRect().height());

  viewProj.setCanvasRect(QRect(0, 0, 100, 200));
  viewProj.setWidgetRect(QRect(0, 0, 100, 200));
  viewProj.setZoom(2);

  QPoint pt = viewProj.mapPointBack(QPointF(10, 10));
  ASSERT_EQ(QPoint(5, 5), pt);

  viewProj.setZoomWithFixedPoint(1, QPoint(10, 20), QPointF(50, 80));
  QPointF pt2 = viewProj.mapPoint(QPoint(10, 20));
  pt = viewProj.mapPointBack(QPointF(50, 80));
  ASSERT_DOUBLE_EQ(50.0, pt2.x());
  ASSERT_DOUBLE_EQ(80.0, pt2.y());
  ASSERT_EQ(QPoint(10, 20), pt);

  viewProj.setOffset(0, 0);
  viewProj.setZoomWithFixedPoint(1, QPoint(10, 20), QPointF(50, 80));
  pt2 = viewProj.mapPoint(QPoint(10, 20));
  pt = viewProj.mapPointBack(QPointF(50, 80));
  ASSERT_DOUBLE_EQ(50.0, pt2.x());
  ASSERT_DOUBLE_EQ(80.0, pt2.y());
  ASSERT_EQ(QPoint(10, 20), pt);

}


#endif

#endif // ZVIEWPROJTEST_H
