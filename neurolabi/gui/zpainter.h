#ifndef ZPAINTER_H
#define ZPAINTER_H

#include "zqtheader.h"

#ifdef _QT_GUI_USED_
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QRect>
#endif

#include "zpoint.h"
#include "zsttransform.h"

class ZIntPoint;
class ZImage;
class ZPixmap;

/*!
 * \brief The painter class using QPainter to draw objects with extended options
 */
class ZPainter : public QPainter
{
public:
  ZPainter();
  explicit ZPainter(QPaintDevice * device);
  explicit ZPainter(ZImage *image);

  bool begin(ZImage *image);
  bool begin(ZPixmap *pixmap);
  bool begin(QPaintDevice *device);


  void setStackOffset(int x, int y, int z);
  void setStackOffset(const ZIntPoint &offset);
  void setStackOffset(const ZPoint &offset);
  void setZOffset(int z);

  inline int getZOffset() { return m_z; }
  //inline ZPoint getOffset() { return m_transform.getOffset(); }

  void drawImage(
      const QRectF &targetRect, const ZImage &image, const QRectF &sourceRect);
  void drawImage(int x, int y, const ZImage &image);

  void drawPixmap(
      const QRectF &targetRect, const ZPixmap &image, const QRectF &sourceRect);
  void drawPixmap(int x, int y, const ZPixmap &image);

#if 0
  void drawPoint(const QPointF &pt);
  void drawPoint(const QPoint &pt);

  void drawPoints(const QPointF *points, int pointCount);
  void drawPoints(const QPoint *points, int pointCount);

  void drawLine(int x1, int y1, int x2, int y2);
  void drawLine(const QPointF &pt1, const QPointF &pt2);
  void	drawEllipse(const QRectF & rectangle);
  void	drawEllipse(const QRect & rectangle);
  void	drawEllipse(int x, int y, int width, int height);
  void	drawEllipse(const QPointF & center, double rx, double ry);
  void	drawEllipse(const QPoint & center, int rx, int ry);

  void	drawRect(const QRectF & rectangle);
  void	drawRect(const QRect & rectangle);
  void	drawRect(int x, int y, int width, int height);

  void	drawPolyline(const QPointF * points, int pointCount);
  void	drawPolyline(const QPoint * points, int pointCount);
#endif
  /*
  const QRect& getFieldOfView() const {
    return m_projRegion;
  }
  */

private:
  int m_z;
  //ZStTransform m_transform; //world coordinates to canvas coordinates
//  ZPoint m_offset;
  //QRect m_projRegion;
};

#endif // ZPAINTER_H
