#ifndef ZPAINTER_H
#define ZPAINTER_H

#include <vector>

//#include "neutube_def.h"
#include "zqtheader.h"

#ifdef _QT_GUI_USED_
#include <QPainter>
#include <QPaintDevice>
#endif

#include "zpoint.h"
#include "zsttransform.h"

class ZIntPoint;
class ZImage;
class ZPixmap;
class QPointF;
class QRectF;
class QRect;
class QTransform;

/*!
 * \brief The painter class using QPainter to draw objects with extended options
 */
class ZPainter
{
public:
  ZPainter();
#ifdef _QT_GUI_USED_
  explicit ZPainter(QPaintDevice * device);
  explicit ZPainter(ZImage *image);
  explicit ZPainter(ZPixmap *pixmap);
#endif
  ~ZPainter();

#ifdef _QT_GUI_USED_
  bool begin(ZImage *image);
  bool begin(ZPixmap *pixmap);
  bool begin(QPaintDevice *device);
  bool end();

  QPaintDevice* device();

  void save();
  void restore();

  bool isActive() const;


  void setStackOffset(int x, int y, int z);
  void setStackOffset(const ZIntPoint &offset);
  void setStackOffset(const ZPoint &offset);
  void setZOffset(int z);

  inline int getZOffset() { return m_z; }

  inline int getZ(int slice) {
    return getZOffset() + slice;
  }

  void setPainted(bool painted) {
    m_isPainted = painted;
  }

  inline bool isPainted() {
    return m_isPainted;
  }

  //inline ZPoint getOffset() { return m_transform.getOffset(); }

  void drawImage(
      const QRectF &targetRect, const ZImage &image, const QRectF &sourceRect);

  /*!
   * \brief Draw image.
   *
   * (\a x, \a y) is the target position in the canvas;
   */
  void drawImage(int x, int y, const ZImage &image);

  /*!
   * \brief Draw image.
   *
   * \a sourceRect is in the world coordinates.
   */
  void drawPixmap(
      const QRectF &targetRect, const ZPixmap &image, const QRectF &sourceRect);

  void drawPixmap(const QRectF &targetRect, const ZPixmap &image);

  /*!
   * \brief Draw image.
   *
   * (\a x, \a y) is the target position in world coordinates.
   */
  void drawPixmap(int x, int y, const ZPixmap &image);
  //ignore transformation
  void drawPixmapNt(const ZPixmap &image);

  void drawPixmap(const ZPixmap &image);

  void drawActivePixmap(
      const QRectF &targetRect, const ZPixmap &image, const QRectF &sourceRect);
  void drawActivePixmap(int x, int y, const ZPixmap &image);


  void setPen(const QColor &color);
  void setPen(const QPen &pen);
  void setPen(Qt::PenStyle style);
  void setFont(const QFont &font);

  void setBrush(const QColor &color);
  void setBrush(const QBrush &pen);
  void setBrush(Qt::BrushStyle style);

  const QBrush& getBrush() const;
  const QPen& getPen() const;
  QColor getPenColor() const;

  const QTransform& getTransform() const;
  void setTransform(const QTransform &t, bool combine = false);

  const ZStTransform& getCanvasTransform() const {
    return m_transform;
  }

  void drawPoint(const QPointF &pt);
  void drawPoint(const QPoint &pt);

  void drawPoints(const QPointF *points, int pointCount);
  void drawPoints(const QPoint *points, int pointCount);
  void drawPoints(const std::vector<QPoint> &pointArray);
  void drawPoints(const std::vector<QPointF> &pointArray);

  void drawLine(int x1, int y1, int x2, int y2);
  void drawLine(const QPointF &pt1, const QPointF &pt2);
  void drawLines(const QLine *lines, int lineCount);
  void drawLines(const std::vector<QLine> &lineArray);

  void drawEllipse(const QRectF & rectangle);
  void drawEllipse(const QRect & rectangle);
  void drawEllipse(int x, int y, int width, int height);
  void drawEllipse(const QPointF & center, double rx, double ry);
  void drawEllipse(const QPoint & center, int rx, int ry);

  void drawRect(const QRectF & rectangle);
  void drawRect(const QRect & rectangle);
  void drawRect(int x, int y, int width, int height);

  void drawArc(const QRectF &rectangle, int startAngle, int spanAngle);

  void drawPolyline(const QPointF * points, int pointCount);
  void drawPolyline(const QPoint * points, int pointCount);

  void drawText(
      int x, int y, int width, int height, int flags, const QString & text);
  void drawStaticText(int x, int y, const QStaticText &text);

  void setCompositionMode(QPainter::CompositionMode mode);
  void setRenderHints(QPainter::RenderHints hints, bool on = true);
  void setRenderHint(QPainter::RenderHint hint, bool on = true);

  void fillRect(const QRect &r, Qt::GlobalColor color);
  void setOpacity(double alpha);
  void setCanvasRange(const QRectF &r) { m_canvasRange = r; }

  bool isVisible(const QRectF &rect) const;
  bool isVisible(const QRect &rect) const;
  bool isVisible(double x1, double y1, double x2, double y2) const;

  QRectF getCanvasRange() const;
#endif

  /*
  const QRect& getFieldOfView() const {
    return m_projRegion;
  }
  */

private:
#ifdef _QT_GUI_USED_
  QPainter m_painter;
#endif

  int m_z;
  QRectF m_canvasRange;
  bool m_isPainted;

  ZStTransform m_transform; //world coordinates to canvas coordinates
//  ZPoint m_offset;
  //QRect m_projRegion;
};

#endif // ZPAINTER_H
