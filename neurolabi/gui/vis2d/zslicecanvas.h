#ifndef ZSLICECANVAS_H
#define ZSLICECANVAS_H

#include <memory>
#include <string>

#include <QPixmap>
#include <QPainter>
#include <QString>

#include "common/zrefcount.h"
#include "geometry/zaffinerect.h"
#include "data3d/zsliceviewtransform.h"
#include "zslicepainter.h"

/*!
 * \brief The class for managing slice painting
 *
 * A slice canvas has two main parts, including the painting buffer (canvas) and
 * the transform from the model space to the canvas space.
 */
class ZSliceCanvas : public ZRefCount
{
public:
  ZSliceCanvas();
  ZSliceCanvas(int width, int height);

  /*!
   * \brief Set the canvas to transparent and unpainted
   */
  void resetCanvas();

  void resetCanvas(int width, int height);
//  void resetCanvas(int width, int height, const ZSliceViewTransform &transform);
  enum class ESetOption {
    NO_CLEAR, //No canvas clearup
    DIFF_CLEAR, //Clear only when parameters changed
    FORCE_CLEAR, //Clear anyway
  };

  enum class ECanvasStatus {
    RAW, //Cavas created but not inited
    CLEAN, //Clean canvas, which is completely transparent
    PAINTED //Painted canvas
  };

  void setCanvasStatus(ECanvasStatus status);
  ECanvasStatus getCanvasStatus() const;

  void set(int width, int height, ESetOption option);
  void set(int width, int height, const ZSliceViewTransform &transform,
           ESetOption option);

  /*!
   * \brief Fit the canvas to the target size
   */
  void fitTarget(int width, int height);

  void setTransform(const ZSliceViewTransform &transform);
  ZSliceViewTransform getTransform() const;
  void setOriginalCut(const ZAffineRect &rect);

  bool paintTo(
      QPaintDevice *device, const ZSliceViewTransform &painterTransform) const;
  bool paintTo(
      QPainter *painter, const ZSliceViewTransform &painterTransform) const;

  /*
  bool paintTo(
      QPaintDevice *device, const ZSliceViewTransform &&painterTransform) const;
  bool paintTo(
      QPainter *painter, const ZSliceViewTransform &&painterTransform) const;
      */

  void setPainted(bool painted);
  void setVisible(bool visible);

  bool isPainted() const;
  bool isVisible() const;
  bool updateNeeded() const;

  QSize getSize() const;
  int getWidth() const;
  int getHeight() const;

  bool isEmpty() const;

  QImage toImage() const;
  void fromImage(const QImage &image);
  void fromImage(const QImage &image, double u0, double v0);

  const QPixmap& getPixmap() const;
  QPixmap* getPixmapRef();

  void save(const char* filePath) const;
  void save(const std::string &filePath) const;
  void save(const QString &filePath) const;

  void clear(const QColor &color);

  friend class ZSliceCanvasPaintHelper;

private:
  void beginPainter(QPainter *painter);

private:
  ZSliceViewTransform m_transform;
  ZAffineRect m_originalCut;
  QPixmap m_pixmap;
  ECanvasStatus m_status = ECanvasStatus::RAW;
  bool m_isVisible = true;
};

/*!
 * \brief The helper class for canvas painting
 *
 * The class helps the program use a correct painter.
 *
 * Usage:
 * ZSliceCanvasPaintHelper p(canvas);
 * QPainter *painter = p.getPainter();
 * ...
 *
 */
class ZSliceCanvasPaintHelper {
public:
  ZSliceCanvasPaintHelper(ZSliceCanvas &canvas);

  QPainter* getPainter();
  ZSlice3dPainter* getSlicePainter();
  const ZSlice3dPainter* getSlicePainter() const;

  void drawBall(double cx, double cy, double cz, double r,
      double depthScale, double fadingFactor);
  void drawBall(const ZPoint &center, double r,
      double depthScale, double fadingFactor);
  void drawBoundBox(double cx, double cy, double cz, double r,
      double depthScale);
  void drawBoundBox(const ZPoint &center, double r, double depthScale);
  void drawLine(const ZLineSegment &line);
  void drawPoint(double x, double y, double z);

  void drawPlanePolyline(
      QPainter *painter, const std::vector<QPointF> &points,
      double z, neutu::EAxis sliceAxis) const;

  void setPen(const QPen &pen);
  void setBrush(const QBrush &brush);

  bool getPaintedHint() const;

private:
  QPainter m_painter;
  ZSlice3dPainter m_slicePainter;
};

#endif // ZSLICECANVAS_H
