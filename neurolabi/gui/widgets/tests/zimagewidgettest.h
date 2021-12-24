#ifndef ZIMAGEWIDGETTEST_H
#define ZIMAGEWIDGETTEST_H

#ifdef _USE_GTEST_

#include <QDebug>

#include "gtest/gtest.h"

#include "geometry/zaffinerect.h"
#include "widgets/zimagewidget.h"

TEST(ZImageWidget, Basic)
{
  ZImageWidget widget(nullptr);
  widget.setModelRange({1, 2, 3, 10, 20, 30});

  ASSERT_EQ(ZIntCuboid(1, 2, 3, 10, 20, 30), widget.getModelRange());

  qDebug() << widget.size();

  {
    std::shared_ptr<ZSliceCanvas> canvas = widget.makeClearCanvas();
    ASSERT_EQ(widget.size(), canvas->getSize());

    ASSERT_FALSE(
          widget.hasCanvas(canvas, neutu::data3d::ETarget::HD_OBJECT_CANVAS));
  }

  {
    auto canvas = widget.getCanvas(
          neutu::data3d::ETarget::HD_OBJECT_CANVAS, false);
    ASSERT_FALSE(canvas);
    canvas = widget.getCanvas(
          neutu::data3d::ETarget::HD_OBJECT_CANVAS, true);
    ASSERT_TRUE(canvas);
    ASSERT_TRUE(canvas->isEmpty());

    widget.validateCanvas(neutu::data3d::ETarget::HD_OBJECT_CANVAS);
    ASSERT_EQ(widget.size(), canvas->getSize());

    ASSERT_TRUE(widget.hasCanvas(
                  canvas, neutu::data3d::ETarget::HD_OBJECT_CANVAS));
    ASSERT_FALSE(widget.hasCanvas(
                  canvas, neutu::data3d::ETarget::PIXEL_OBJECT_CANVAS));

    ASSERT_TRUE(canvas->isVisible());
    widget.setCanvasVisible(neutu::data3d::ETarget::HD_OBJECT_CANVAS, false);
    ASSERT_FALSE(canvas->isVisible());
  }

  ZSliceViewTransform transform = widget.getSliceViewTransform();
  std::cout << transform << std::endl;

  widget.setCutCenter(10, 20, 30, neutu::ESignalControl::SLIENT);
  ASSERT_EQ(ZPoint(10, 20, 30), widget.getCutCenter());

  widget.setSliceAxis(neutu::EAxis::X, neutu::ESignalControl::SLIENT);
  ASSERT_EQ(ZPoint(10, 20, 30), widget.getCutCenter());

  widget.setCutCenter(10.5, 20, 30, neutu::ESignalControl::SLIENT);
  widget.setSliceAxis(neutu::EAxis::ARB, neutu::ESignalControl::SLIENT);
  ASSERT_EQ(ZPoint(10.5, 20, 30), widget.getCutCenter());

  widget.setCutPlane(neutu::EAxis::X, neutu::ESignalControl::SLIENT);
  ASSERT_EQ(ZPoint(10, 20, 30), widget.getCutCenter());

  widget.setCutCenter(3.5, 4.5, 5.5, neutu::ESignalControl::SLIENT);
  ASSERT_EQ(ZPoint(3.5, 4.5, 5.5), widget.getCutCenter());

  widget.setCutPlane(neutu::EAxis::Y, neutu::ESignalControl::SLIENT);
  ASSERT_EQ(ZPoint(3.5, 5, 5.5), widget.getCutCenter());

  widget.moveCutDepth(1.0, neutu::ESignalControl::SLIENT);
  ASSERT_EQ(ZPoint(3.5, 6, 5.5), widget.getCutCenter());

  qDebug() << widget.getAnchorPoint();
}

TEST(ZImageWidget, viewport)
{
  ZImageWidget widget(nullptr);
  widget.setModelRange({1, 2, 3, 10, 20, 30});
  widget.resetView(1.0);
  widget.setCutCenter(5, 6, 7, neutu::ESignalControl::SLIENT);

  ZAffineRect rect = widget.getViewRegion();
  ASSERT_EQ(ZPoint(5, 6, 7), rect.getCenter());

  widget.resetView(1.0);

  std::cout << widget.getViewRegion() << std::endl;
}

#endif

#endif // ZIMAGEWIDGETTEST_H
