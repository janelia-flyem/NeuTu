#ifndef ZVIEWPLANETRANSFORMTEST_H
#define ZVIEWPLANETRANSFORMTEST_H

#include "data3d/zviewplanetransform.h"
#include "geometry/zpoint.h"

#ifdef _USE_GTEST_

#include "gtest/gtest.h"

TEST(ZViewPlaneTransform, Basic)
{
  {
    ZViewPlaneTransform t;
    t.set(1, 2, 3);
    ASSERT_EQ(1.0, t.getTx());
    ASSERT_EQ(2.0, t.getTy());
    ASSERT_EQ(3.0, t.getScale());

    t.setScale(5);
    ASSERT_EQ(5.0, t.getScale());
    t.setOffset(2, 1);
    ASSERT_EQ(2.0, t.getTx());
    ASSERT_EQ(1.0, t.getTy());

    t.incScale();
    ASSERT_DOUBLE_EQ(5.5, t.getScale());

    t.decScale();
    ASSERT_DOUBLE_EQ(5.0, t.getScale());

    ASSERT_EQ(t, t);

    ASSERT_TRUE(t.canZoomOut());
    t.setMinScale(0.5);
    t.setScale(0.1);
    ASSERT_FALSE(t.canZoomOut());
    ASSERT_EQ(t.getScale(), t.getMinScale());
    t.setMaxScale(10.0);
    ASSERT_TRUE(t.canZoomIn());
    t.setScale(11);
    ASSERT_FALSE(t.canZoomIn());
    ASSERT_EQ(t.getScale(), t.getMaxScale());
  }
}

TEST(ZViewPlaneTransform, Transform)
{
  {
    ZViewPlaneTransform t;
    t.set(0, 0, 1);

    double x = 1.0;
    double y = 2.0;
    t.transform(&x, &y);
    ASSERT_EQ(1.0, x);
    ASSERT_EQ(2.0, y);

    t.setOffset(10, 20);
    t.transform(&x, &y);
    ASSERT_EQ(11, x);
    ASSERT_EQ(22, y);

    x = 1.0;
    y = 2.0;
    t.setScale(2.0);
    t.transform(&x, &y);
    ASSERT_EQ(12, x);
    ASSERT_EQ(24, y);

    t.inverseTransform(&x, &y);
    ASSERT_EQ(1.0, x);
    ASSERT_EQ(2.0, y);

    ZPoint pt(1, 2, 3);
    ZPoint newPt = t.transform(pt);
    ASSERT_EQ(12, newPt.getX());
    ASSERT_EQ(24, newPt.getY());
    ASSERT_EQ(3, newPt.getZ());

    neutu::geom2d::Rectangle rect(1, 2, 10, 20);
    neutu::geom2d::Rectangle r2 = t.inverseTransform(t.transform(rect));
    ASSERT_EQ(rect, r2);

  }
}

TEST(ZViewPlaneTransform, Set)
{
  {
    ZViewPlaneTransform t;
    t.centerFit(3, 5, 10, 20, 20, 40);

    neutu::geom2d::Rectangle rect;
    rect.setCenter(3, 5, 10, 20);
    neutu::geom2d::Rectangle r2 = t.transform(rect);
    ASSERT_EQ(r2, neutu::geom2d::Rectangle(0, 0, 20, 40));

    neutu::geom2d::Rectangle targetRect;
    targetRect.setCenter(3, 5, 10, 20);
    t.centerFit(rect, targetRect);
    ASSERT_EQ(1.0, t.getScale());
    ASSERT_EQ(0.0, t.getTx());
    ASSERT_EQ(0.0, t.getTy());

    targetRect.setCenter(4, 7, 10, 20);
    t.centerFit(rect, targetRect);
    ASSERT_EQ(1.0, t.getScale());
    ASSERT_EQ(1.0, t.getTx());
    ASSERT_EQ(2.0, t.getTy());

    targetRect.setCenter(4, 7, 50, 40);
    t.centerFit(rect, targetRect);
    ASSERT_EQ(2.0, t.getScale());
    ASSERT_EQ(-2.0, t.getTx());
    ASSERT_EQ(-3.0, t.getTy());
    r2 = t.transform(rect);
    ASSERT_EQ(r2.getCenter(), targetRect.getCenter());
    ASSERT_EQ(r2.getHeight(), targetRect.getHeight());

    rect.setCenter(4, 6, 10, 20);
    targetRect.setCenter(4, 7, 5, 30);
    t.centerFit(rect, targetRect);
    ASSERT_DOUBLE_EQ(0.5, t.getScale());
    ASSERT_EQ(2.0, t.getTx());
    ASSERT_EQ(4.0, t.getTy());
    r2 = t.transform(rect);
    ASSERT_EQ(r2.getCenter(), targetRect.getCenter());
    ASSERT_EQ(r2.getWidth(), targetRect.getWidth());

    neutu::geom2d::Point pt(3.0, 4.0);
    auto pt2 = t.transform(pt);
    t.setScaleFixingOriginal(0.1, pt.getX(), pt.getY());
    ASSERT_EQ(0.1, t.getScale());
    ASSERT_EQ(pt2, t.transform(pt));

    t.setScaleFixingMapped(10.0, pt2.getX(), pt2.getY());
    ASSERT_EQ(10.0, t.getScale());
    auto pt3 = t.inverseTransform(pt2);
    ASSERT_DOUBLE_EQ(pt.getX(), pt3.getX());
    ASSERT_DOUBLE_EQ(pt.getY(), pt3.getY());

    t.setScale(5.0);
    t.translateTransform(pt, pt2);
    pt3 = t.transform(pt);
    ASSERT_DOUBLE_EQ(pt2.getX(), pt3.getX());
    ASSERT_DOUBLE_EQ(pt2.getY(), pt3.getY());

  }
}

#endif

#endif // ZVIEWPLANETRANSFORMTEST_H
