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

/*!
 * \brief The painter class using QPainter to draw objects with extended options
 */
class ZPainter : public QPainter
{
public:
  ZPainter();
  ZPainter(QPaintDevice * device);
  inline void setStackOffset(double x, double y, double z) {
    m_offset.set(-x, -y, -z);
  }
  inline void setStackOffset(const ZPoint &offset) {
    setStackOffset(offset.x(), offset.y(), offset.z());
  }

  inline const ZPoint& getOffset() { return m_offset; }

  void drawLine(int x1, int y1, int x2, int y2);
  void drawLine(const QPointF &pt1, const QPointF &pt2);
  void	drawEllipse(const QRectF & rectangle);
  void	drawEllipse(const QRect & rectangle);
  void	drawEllipse(int x, int y, int width, int height);
  void	drawEllipse(const QPointF & center, qreal rx, qreal ry);
  void	drawEllipse(const QPoint & center, int rx, int ry);

  void	drawRect(const QRectF & rectangle);
  void	drawRect(const QRect & rectangle);
  void	drawRect(int x, int y, int width, int height);

  void	drawPolyline(const QPointF * points, int pointCount);
  void	drawPolyline(const QPoint * points, int pointCount);

private:
  ZPoint m_offset;
};

#endif // ZPAINTER_H
