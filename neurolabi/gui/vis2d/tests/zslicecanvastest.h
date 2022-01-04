#ifndef ZSLICECANVASTEST_H
#define ZSLICECANVASTEST_H

#if _USE_GTEST_

#include <QPixmap>
#include <QImage>

#include "test/ztestheader.h"
#include "../zslicecanvas.h"

namespace {

bool is_transparent(const QPixmap &pixmap)
{
  QImage image = pixmap.toImage();
  for (int y = 0; y < pixmap.height(); ++y) {
    for (int x = 0; x < pixmap.width(); ++x) {
      QColor color = image.pixelColor(x, y);
      if (color.alpha() != 0) {
        return false;
      }
    }
  }
  return true;
}

}

TEST(ZSliceCanvas, Constructor)
{
  {
    ZSliceCanvas canvas;
    ASSERT_TRUE(canvas.isEmpty());
  }

  {
    ZSliceCanvas canvas(800, 600);
    ASSERT_FALSE(canvas.isEmpty());
    ASSERT_EQ(800, canvas.getWidth());
    ASSERT_EQ(600, canvas.getHeight());
  }
}

TEST(ZSliceCanvas, Properties)
{
  {
    ZSliceCanvas canvas;
    canvas.resetCanvas(80, 60);
    ASSERT_FALSE(canvas.isEmpty());
    ASSERT_EQ(80, canvas.getWidth());
    ASSERT_EQ(60, canvas.getHeight());
    ASSERT_TRUE(is_transparent(canvas.getPixmap()));
    std::cout << canvas.getTransform() << std::endl;

    ASSERT_FALSE(canvas.isPainted());
    canvas.setPainted(true);
    ASSERT_TRUE(canvas.isPainted());
  }
}

TEST(ZSliceCanvas, Paint)
{
  ZSliceCanvas canvas(80, 60);
  ZSliceCanvasPaintHelper helper(canvas);
  helper.setPen(QPen(Qt::red));
  helper.drawPoint(0, 0, 0);
  ASSERT_FALSE(is_transparent(canvas.getPixmap()));

  const QPixmap& pixmap = canvas.getPixmap();
  QImage image = pixmap.toImage();
  QColor color = image.pixelColor(0, 0);
  ASSERT_EQ(QColor(255, 0, 0, 255), color);
  ASSERT_EQ(QColor(0, 0, 0, 0), image.pixelColor(1, 0));

  image = canvas.toImage();
  ASSERT_EQ(QColor(255, 0, 0, 255), color);
  ASSERT_EQ(QColor(0, 0, 0, 0), image.pixelColor(1, 0));

}

#endif

#endif // ZSLICECANVASTEST_H
