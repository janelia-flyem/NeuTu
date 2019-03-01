#ifndef ZPOSITIONMAPPERTEST_H
#define ZPOSITIONMAPPERTEST_H

#include "ztestheader.h"
#include "mvc/zpositionmapper.h"
#include "zviewproj.h"
#include "mvc/zstackdoc.h"
#include "geometry/zaffineplane.h"
#include "mvc/zstackdocutil.h"

#ifdef _USE_GTEST_

TEST(ZPositionMapper, WidgetToRawStack)
{
  ZStackDoc doc;
  doc.loadStack(ZStackFactory::MakeVirtualStack(100, 200, 300));
  ZIntCuboid box = ZStackDocUtil::GetStackSpaceRange(&doc, neutu::EAxis::Z);

  ZViewProj viewProj;
  viewProj.setCanvasRect(QRect(box.getFirstCorner().getX(),
                               box.getFirstCorner().getY(),
                               box.getWidth(), box.getHeight()));
  viewProj.setWidgetRect(QRect(0, 0, 100, 200));

  double x = 50.0;
  double y = 50.0;
  QPointF pt = ZPositionMapper::WidgetToRawStack(QPointF(x, y), viewProj);
//  processor.mapPositionFromWidgetToRawStack(&x, &y, viewProj);
  ASSERT_DOUBLE_EQ(50.0, pt.x());
  ASSERT_DOUBLE_EQ(50.0, pt.y());

  viewProj.setWidgetRect(QRect(0, 0, 200, 400));
  pt = ZPositionMapper::WidgetToRawStack(QPointF(x, y), viewProj);
  ASSERT_DOUBLE_EQ(50.0, pt.x());
  ASSERT_DOUBLE_EQ(50.0, pt.y());

  viewProj.maximizeViewPort();
  pt = ZPositionMapper::WidgetToRawStack(QPointF(x, y), viewProj);
  ASSERT_DOUBLE_EQ(25.0, pt.x());
  ASSERT_DOUBLE_EQ(25.0, pt.y());

  viewProj.setWidgetRect(QRect(0, 0, 50, 100));
  viewProj.maximizeViewPort();
  pt = ZPositionMapper::WidgetToRawStack(QPointF(10, 20), viewProj);
  ASSERT_DOUBLE_EQ(20, pt.x());
  ASSERT_DOUBLE_EQ(40, pt.y());

  ZPoint pt2 = ZPositionMapper::WidgetToRawStack(
        ZIntPoint(10, 20, 30), viewProj);
  ASSERT_DOUBLE_EQ(20, pt2.getX());
  ASSERT_DOUBLE_EQ(40, pt2.getY());
  ASSERT_DOUBLE_EQ(30, pt2.getZ());
}

TEST(ZPositionMapper, WidgetToRawStackWithOffset)
{
  ZStackDoc doc;
  ZIntCuboid range;
  range.setFirstCorner(10, 20, 30);
  range.setSize(100, 200, 300);
  doc.loadStack(ZStackFactory::MakeVirtualStack(range));
  ZIntCuboid box = ZStackDocUtil::GetStackSpaceRange(&doc, neutu::EAxis::Z);

  ZViewProj viewProj;
  viewProj.setCanvasRect(QRect(box.getFirstCorner().getX(),
                               box.getFirstCorner().getY(),
                               box.getWidth(), box.getHeight()));
  viewProj.setWidgetRect(QRect(0, 0, 100, 200));

  double x = 50.0;
  double y = 50.0;
  QPointF pt = ZPositionMapper::WidgetToRawStack(QPointF(x, y), viewProj);
//  processor.mapPositionFromWidgetToRawStack(&x, &y, viewProj);
  ASSERT_DOUBLE_EQ(40.0, pt.x());
  ASSERT_DOUBLE_EQ(30.0, pt.y());

  viewProj.setWidgetRect(QRect(0, 0, 200, 400));
  pt = ZPositionMapper::WidgetToRawStack(QPointF(x, y), viewProj);
  ASSERT_DOUBLE_EQ(40.0, pt.x());
  ASSERT_DOUBLE_EQ(30.0, pt.y());

  viewProj.maximizeViewPort();
  pt = ZPositionMapper::WidgetToRawStack(QPointF(x, y), viewProj);
  ASSERT_DOUBLE_EQ(25.0, pt.x());
  ASSERT_DOUBLE_EQ(25.0, pt.y());

  viewProj.setWidgetRect(QRect(0, 0, 50, 100));
  viewProj.maximizeViewPort();
  pt = ZPositionMapper::WidgetToRawStack(QPointF(10, 20), viewProj);
  ASSERT_DOUBLE_EQ(20, pt.x());
  ASSERT_DOUBLE_EQ(40, pt.y());

  ZPoint pt2 = ZPositionMapper::WidgetToRawStack(
        ZIntPoint(10, 20, 30), viewProj);
  ASSERT_DOUBLE_EQ(20, pt2.getX());
  ASSERT_DOUBLE_EQ(40, pt2.getY());
  ASSERT_DOUBLE_EQ(30, pt2.getZ());
}

TEST(ZPositionMapper, WidgetToStack)
{
  ZStackDoc doc;
  ZIntCuboid range;
  range.setFirstCorner(10, 20, 30);
  range.setSize(100, 200, 300);
  doc.loadStack(ZStackFactory::MakeVirtualStack(range));
  ZIntCuboid box = ZStackDocUtil::GetStackSpaceRange(&doc, neutu::EAxis::Z);

  ZViewProj viewProj;
  viewProj.setCanvasRect(QRect(box.getFirstCorner().getX(),
                               box.getFirstCorner().getY(),
                               box.getWidth(), box.getHeight()));
  viewProj.setWidgetRect(QRect(0, 0, 100, 200));

  double x = 50.0;
  double y = 50.0;
  QPointF pt = ZPositionMapper::WidgetToStack(QPointF(x, y), viewProj);
//  processor.mapPositionFromWidgetToRawStack(&x, &y, viewProj);
  ASSERT_DOUBLE_EQ(50.0, pt.x());
  ASSERT_DOUBLE_EQ(50.0, pt.y());

  viewProj.setWidgetRect(QRect(0, 0, 200, 400));
  pt = ZPositionMapper::WidgetToStack(QPointF(x, y), viewProj);
  ASSERT_DOUBLE_EQ(50.0, pt.x());
  ASSERT_DOUBLE_EQ(50.0, pt.y());

  viewProj.maximizeViewPort();
  pt = ZPositionMapper::WidgetToStack(QPointF(x, y), viewProj);
  ASSERT_DOUBLE_EQ(35.0, pt.x());
  ASSERT_DOUBLE_EQ(45.0, pt.y());

  viewProj.setWidgetRect(QRect(0, 0, 50, 100));
  viewProj.maximizeViewPort();
  pt = ZPositionMapper::WidgetToStack(QPointF(10, 20), viewProj);
  ASSERT_DOUBLE_EQ(30, pt.x());
  ASSERT_DOUBLE_EQ(60, pt.y());

  ZPoint pt2 = ZPositionMapper::WidgetToStack(
        ZPoint(10, 20, 30), viewProj, box.getFirstCorner().getZ());
  ASSERT_DOUBLE_EQ(30, pt2.getX());
  ASSERT_DOUBLE_EQ(60, pt2.getY());
  ASSERT_DOUBLE_EQ(60, pt2.getZ());

  pt2 = ZPositionMapper::WidgetToStack(
        10, 20, viewProj, box.getFirstCorner().getZ());
  ASSERT_DOUBLE_EQ(30, pt2.getX());
  ASSERT_DOUBLE_EQ(60, pt2.getY());
  ASSERT_DOUBLE_EQ(30, pt2.getZ());
}

TEST(ZPositionMapper, StackToData)
{
  ZAffinePlane ap;
  ap.setOffset(ZPoint(100, 200, 300));
  ZPoint pt = ZPositionMapper::StackToData(ZPoint(10, 20, 30), ap);
  ASSERT_DOUBLE_EQ(10, pt.getX());
  ASSERT_DOUBLE_EQ(20, pt.getY());
  ASSERT_DOUBLE_EQ(30, pt.getZ());

  pt = ZPositionMapper::StackToData(ZPoint(10, 20, 30), ZPoint(0, 0, 0), ap);
  ASSERT_DOUBLE_EQ(110, pt.getX());
  ASSERT_DOUBLE_EQ(220, pt.getY());
  ASSERT_DOUBLE_EQ(330, pt.getZ());

  ap.setPlane(ZPoint(0, 1, 0), ZPoint(0, 0, 1));
  pt = ZPositionMapper::StackToData(ZPoint(10, 20, 30), ap);
  ASSERT_DOUBLE_EQ(-170, pt.getX());
  ASSERT_DOUBLE_EQ(110, pt.getY());
  ASSERT_DOUBLE_EQ(120, pt.getZ());

  pt = ZPositionMapper::StackToData(ZPoint(10, 20, 30), ZPoint(0, 0, 0), ap);
  ASSERT_DOUBLE_EQ(130, pt.getX());
  ASSERT_DOUBLE_EQ(210, pt.getY());
  ASSERT_DOUBLE_EQ(320, pt.getZ());

  pt = ZPositionMapper::StackToData(ZPoint(10, 20, 30), neutu::EAxis::X);
  ASSERT_DOUBLE_EQ(30, pt.getX());
  ASSERT_DOUBLE_EQ(20, pt.getY());
  ASSERT_DOUBLE_EQ(10, pt.getZ());

  ap.setPlane(ZPoint(std::sqrt(0.5), -std::sqrt(0.5), 0),
              ZPoint(std::sqrt(0.5), std::sqrt(0.5), 0));
  pt = ZPositionMapper::StackToData(ZPoint(10, 20, 30), ap);
  ASSERT_DOUBLE_EQ(-90.918830920367839, pt.getX());
  ASSERT_DOUBLE_EQ(136.36038969321072, pt.getY());
  ASSERT_DOUBLE_EQ(29.999999999999943, pt.getZ());
}

#endif

#endif // ZPOSITIONMAPPERTEST_H
