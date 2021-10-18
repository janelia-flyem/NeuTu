#include "utilities.h"

#include <memory>
#include <QtGui>

#include "common/math.h"
#include "common/utilities.h"

#include "zstack.hxx"
#include "zsparsestack.h"
#include "zimage.h"
#include "data3d/displayconfig.h"
#include "data3d/zviewplanetransform.h"
#include "zslicepainter.h"
#include "zslicecanvas.h"

QTransform neutu::vis2d::GetPainterTransform(const ZViewPlaneTransform &t)
{
  QTransform transform;
  transform.setMatrix(
        t.getScale(), 0, 0, 0, t.getScale(), 0, t.getTx(), t.getTy(), 1);

  return  transform;
}

ZSlice3dPainter neutu::vis2d::Get3dSlicePainter(
    const neutu::data3d::DisplayConfig &config)
{
  ZSlice3dPainter painter;

  painter.setModelViewTransform(config.getWorldViewTransform());
  painter.setViewCanvasTransform(config.getViewCanvasTransform());

  return painter;
}

ZSliceViewTransform GetTransform(const neutu::data3d::DisplayConfig &config)
{
  return config.getTransform();
}

QImage neutu::vis2d::GetSlice(const ZStack &stack, int z)
{
  int x0 = stack.getOffset().getX();
  int y0 = stack.getOffset().getY();
  int x1 = x0 + stack.width() - 1;
  int y1 = y0 + stack.height() - 1;
  return GetSlice(stack, x0, y0, x1, y1, z);
}

QImage neutu::vis2d::GetSlice(const ZSparseStack &stack, int z)
{
  ZIntCuboid box = stack.getBoundBox();
  int x0 = box.getMinCorner().getX();
  int y0 = box.getMaxCorner().getY();
  int x1 = x0 + box.getWidth() - 1;
  int y1 = y0 + box.getHeight() - 1;
  return GetSlice(stack, x0, y0, x1, y1, z);
}

QImage neutu::vis2d::GetSlice(
    const ZStack &stack, int &x0, int &y0, int &x1, int &y1, int z)
{
  ZIntCuboid box = stack.getBoundBox();

  if (neutu::WithinCloseRange(z, box.getMinZ(), box.getMaxZ())) {
    x0 = std::max(box.getMinX(), x0);
    x1 = std::min(box.getMaxX(), x1);
    y0 = std::max(box.getMinY(), y0);
    y1 = std::min(box.getMaxY(), y1);

#ifdef _DEBUG_
    std::cout << "Slice Range: (" << x0 << ", " << y0 << "), ("
              << x1 << ", " << y1 << ")" << std::endl;
#endif

    if (x0 < x1 && y0 < y1) {
      int width = x1 - x0 + 1;
      int height = y1 - y0 + 1;
      QImage image(width, height, QImage::Format_Indexed8);
      for (int i = 0; i < 256; ++i) {
        image.setColor(i, qRgb(i, i, i));
      }
      uchar *line = nullptr;
      const uint8_t *data = stack.getDataPointer(x0, y0, z);
      for (int y = 0; y < height; ++y) {
        line = image.scanLine(y);
        for (int x = 0; x < width; ++x) {
          *line++ = data[x];
        }
        data += stack.width();
      }

      return image;
    }
  }

  return QImage();
}

QImage neutu::vis2d::GetSlice(
    const ZSparseStack &stack, int &x0, int &y0, int &x1, int &y1, int z)
{
  ZIntCuboid box = stack.getBoundBox();

  if (neutu::WithinCloseRange(z, box.getMinZ(), box.getMaxZ())) {
    std::shared_ptr<ZStack> slice{stack.getSlice(z)};
    if (slice) {
      return GetSlice(*slice, x0, y0, x1, y1, z);
    }
  }

  return QImage();
}


//ZY plane
QImage neutu::vis2d::GetSliceX(
    const ZStack &stack, int &x0, int &y0, int &x1, int &y1, int z)
{
  ZIntCuboid box = stack.getBoundBox();

  if (neutu::WithinCloseRange(z, box.getMinX(), box.getMaxX())) {
    x0 = std::max(box.getMinZ(), x0);
    x1 = std::min(box.getMaxZ(), x1);
    y0 = std::max(box.getMinY(), y0);
    y1 = std::min(box.getMaxY(), y1);

    if (x0 < x1 && y0 < y1) {
      int width = x1 - x0 + 1;
      int height = y1 - y0 + 1;
      QImage image(width, height, QImage::Format_Indexed8);
      for (int i = 0; i < 256; ++i) {
        image.setColor(i, qRgb(i, i, i));
      }
      uchar *line = nullptr;
      const uint8_t *data = stack.getDataPointer(z, y0, x0);
      int area = stack.width() * stack.height();
      for (int y = 0; y < height; ++y) {
        line = image.scanLine(y);
        for (int x = 0; x < width; ++x) {
          *line++ = data[x * area];
        }
        data += stack.width();
      }

      return image;
    }
  }

  return QImage();
}

//XZ plane
QImage neutu::vis2d::GetSliceY(
    const ZStack &stack, int &x0, int &y0, int &x1, int &y1, int z)
{
  ZIntCuboid box = stack.getBoundBox();

  if (neutu::WithinCloseRange(z, box.getMinY(), box.getMaxY())) {
    x0 = std::max(box.getMinX(), x0);
    x1 = std::min(box.getMaxX(), x1);
    y0 = std::max(box.getMinZ(), y0);
    y1 = std::min(box.getMaxZ(), y1);

    if (x0 < x1 && y0 < y1) {
      int width = x1 - x0 + 1;
      int height = y1 - y0 + 1;
      QImage image(width, height, QImage::Format_Indexed8);
      for (int i = 0; i < 256; ++i) {
        image.setColor(i, qRgb(i, i, i));
      }
      uchar *line = nullptr;
      const uint8_t *data = stack.getDataPointer(x0, z, y0);
      int area = stack.width() * stack.height();
      for (int y = 0; y < height; ++y) {
        line = image.scanLine(y);
        for (int x = 0; x < width; ++x) {
          *line++ = data[x];
        }
        data += area;
      }

      return image;
    }
  }

  return QImage();
}

QImage neutu::vis2d::GetSlice(const ZStack &stack, ZAffineRect &rect)
{
  QImage image;

  if (stack.kind() == GREY) {
//    double halfWidth = (rect.getWidth() - 1.0) / 2.0;
//    double halfHeight = (rect.getHeight() - 1.0) / 2.0;
    double halfWidth = rect.getWidth() / 2.0;
    double halfHeight = rect.getHeight() / 2.0;
    int x0 = 0;
    int y0 = 0;
    int x1 = 0;
    int y1 = 0;
    int z = 0;
    double cx = 0;
    double cy = 0;
    int cz = 0;

    ZIntCuboid box = stack.getBoundBox();
    if (rect.getV1() == ZPoint(1, 0, 0) && rect.getV2() == ZPoint(0, 1, 0)) {
      z = neutu::iround(rect.getCenter().getZ());
      if (neutu::WithinCloseRange(z, box.getMinZ(), box.getMaxZ())) {
        x0 = neutu::ifloor(rect.getCenter().getX() - halfWidth);
        x1 = neutu::iround(rect.getCenter().getX() + halfWidth);
        y0 = neutu::ifloor(rect.getCenter().getY() - halfHeight);
        y1 = neutu::iround(rect.getCenter().getY() + halfHeight);
        image = GetSlice(stack, x0, y0, x1, y1, z);
        cx = (x0 + x1 + 1) / 2.0;
        cy = (y0 + y1 + 1) / 2.0;
        cz = z;
      }
    } else if (rect.getV1() == ZPoint(0, 0, 1) &&
               rect.getV2() == ZPoint(0, 1, 0)) { //along X
      z = neutu::iround(rect.getCenter().getX());
      if (neutu::WithinCloseRange(z, box.getMinX(), box.getMaxX())) {
        x0 = neutu::ifloor(rect.getCenter().getZ() - halfWidth);
        x1 = neutu::iround(rect.getCenter().getZ() + halfWidth);
        y0 = neutu::ifloor(rect.getCenter().getY() - halfHeight);
        y1 = neutu::iround(rect.getCenter().getY() + halfHeight);
        image = GetSliceX(stack, x0, y0, x1, y1, z);
        cz = (x0 + x1 + 1) / 2.0;
        cy = (y0 + y1 + 1) / 2.0;
        cx = z;
      }
    } else if (rect.getV1() == ZPoint(1, 0, 0) &&
               rect.getV2() == ZPoint(0, 0, 1)) { //along Y
      z = neutu::iround(rect.getCenter().getY());
      if (neutu::WithinCloseRange(z, box.getMinY(), box.getMaxY())) {
        x0 = neutu::ifloor(rect.getCenter().getX() - halfWidth);
        x1 = neutu::iround(rect.getCenter().getX() + halfWidth);
        y0 = neutu::ifloor(rect.getCenter().getZ() - halfHeight);
        y1 = neutu::iround(rect.getCenter().getZ() + halfHeight);
        image = GetSliceY(stack, x0, y0, x1, y1, z);
        cx = (x0 + x1 + 1) / 2.0;
        cz = (y0 + y1 + 1) / 2.0;
        cy = z;
      }
    }

    if (!image.isNull()) {
      rect.setSize(image.width(), image.height());
      rect.setCenter(cx, cy, cz);
    }
  }

  return image;
}

QImage neutu::vis2d::GetSlice(const ZSparseStack &stack, ZAffineRect &rect)
{
  QImage image;

  double halfWidth = rect.getWidth() / 2.0;
  double halfHeight = rect.getHeight() / 2.0;
  int x0 = 0;
  int y0 = 0;
  int x1 = 0;
  int y1 = 0;
  int z = 0;
  double cx = 0;
  double cy = 0;
  int cz = 0;

  ZIntCuboid box = stack.getBoundBox();
  if (rect.getV1() == ZPoint(1, 0, 0) && rect.getV2() == ZPoint(0, 1, 0)) {
    z = neutu::iround(rect.getCenter().getZ());
    if (neutu::WithinCloseRange(z, box.getMinZ(), box.getMaxZ())) {
      x0 = neutu::ifloor(rect.getCenter().getX() - halfWidth);
      x1 = neutu::iround(rect.getCenter().getX() + halfWidth);
      y0 = neutu::ifloor(rect.getCenter().getY() - halfHeight);
      y1 = neutu::iround(rect.getCenter().getY() + halfHeight);
      image = GetSlice(stack, x0, y0, x1, y1, z);
      cx = (x0 + x1 + 1) / 2.0;
      cy = (y0 + y1 + 1) / 2.0;
      cz = z;
    }
  }

  if (!image.isNull()) {
    rect.setSize(image.width(), image.height());
    rect.setCenter(cx, cy, cz);
  }

  return image;
}

/*
ZSliceCanvas neutu::vis2d::GetSliceCanvas(
    const ZStack &stack, const ZAffineRect &rect)
{
  ZAffineRect newRect = rect;
  QImage image = GetSlice(stack, newRect);

  ZSliceCanvas canvas;
  ZSliceViewTransform t;
  t.setCutPlane(neutu::EAxis::ARB, newRect.getCenter());
//  t.setAnchor((newRect.getWidth() - 1) / 2.0, (newRect.getHeight() - 1) / 2.0);
  t.setAnchor(neutu::ifloor((newRect.getWidth() - 1.0) / 2.0),
              neutu::ifloor((newRect.getHeight() - 1.0) / 2.0));
  canvas.setTransform(t);
  canvas.fromImage(image);

  return canvas;
}
*/
