#include "zpainter.h"

ZPainter::ZPainter()
{

}

ZPainter::ZPainter(QPaintDevice *device) : QPainter(device)
{
}

void ZPainter::drawLine(int x1, int y1, int x2, int y2)
{
  drawLine(QPointF(x1, y1), QPointF(x2, y2));
}

void ZPainter::drawLine(const QPointF &pt1, const QPointF &pt2)
{
#if _QT_GUI_USED_
  QPainter::drawLine(pt1 + QPointF(m_offset.x(), m_offset.y()),
                     pt2 + QPointF(m_offset.x(), m_offset.y()));
#endif
}

void ZPainter::drawEllipse(const QRectF & rectangle)
{
#if _QT_GUI_USED_
  QRectF rect = rectangle;
  rect.moveCenter(QPointF(m_offset.x(), m_offset.y()));
  QPainter::drawEllipse(rect);
#endif
}

void ZPainter::drawEllipse(const QRect & rectangle)
{
  drawEllipse(QRectF(rectangle));
}

void ZPainter::drawEllipse(int x, int y, int width, int height)
{
  drawEllipse(QPointF(x, y), width, height);
}

void ZPainter::drawEllipse(const QPointF & center, qreal rx, qreal ry)
{
#if _QT_GUI_USED_
  QPointF newCenter = QPointF(m_offset.x(), m_offset.y());
  newCenter += center;
  QPainter::drawEllipse(newCenter, rx, ry);
#endif
}

void ZPainter::drawEllipse(const QPoint & center, int rx, int ry)
{
  drawEllipse(QPointF(center), rx, ry);
}

void ZPainter::drawRect(const QRectF & rectangle)
{
#if _QT_GUI_USED_
  QRectF rect = rectangle;
  rect.moveCenter(QPointF(m_offset.x(), m_offset.y()));
  QPainter::drawRect(rect);
#endif
}

void ZPainter::drawRect(const QRect & rectangle)
{
  drawRect(QRectF(rectangle));
}

void ZPainter::drawRect(int x, int y, int width, int height)
{
  drawRect(QRectF(x, y, width, height));
}

void ZPainter::drawPolyline(const QPointF * points, int pointCount)
{
#ifdef _QT_GUI_USED_
  QPointF *newPoints = new QPointF[pointCount];
  for (int i = 0; i < pointCount; ++i) {
    newPoints[i].setX(points[i].x() + m_offset.x());
    newPoints[i].setY(points[i].y() + m_offset.y());
  }
  QPainter::drawPolyline(newPoints, pointCount);
  delete []newPoints;
#endif
}

void ZPainter::drawPolyline(const QPoint * points, int pointCount)
{
#ifdef _QT_GUI_USED_
  QPointF *newPoints = new QPointF[pointCount];
  for (int i = 0; i < pointCount; ++i) {
    newPoints[i].setX(points[i].x() + m_offset.x());
    newPoints[i].setY(points[i].y() + m_offset.y());
  }
  QPainter::drawPolyline(newPoints, pointCount);
  delete []newPoints;
#endif
}
