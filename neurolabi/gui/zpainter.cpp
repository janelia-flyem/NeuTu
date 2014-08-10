#include "zpainter.h"
#include "zintpoint.h"

ZPainter::ZPainter()
{

}

ZPainter::ZPainter(QPaintDevice *device) : QPainter(device)
{
}

void ZPainter::setStackOffset(int x, int y, int z)
{
  m_offset.set(-x, -y, -z);
}

void ZPainter::setStackOffset(const ZIntPoint &offset)
{
  setStackOffset(offset.getX(), offset.getY(), offset.getZ());
}

void ZPainter::setStackOffset(const ZPoint &offset)
{
  m_offset.set(offset.x(), offset.y(), offset.z());
}

void ZPainter::drawPoint(const QPointF &pt)
{
  QPainter::drawPoint(pt + QPointF(m_offset.x(), m_offset.y()));
}

void ZPainter::drawPoint(const QPoint &pt)
{
  QPainter::drawPoint(pt + QPointF(m_offset.x(), m_offset.y()));
}

void ZPainter::drawPoints(const QPointF *points, int pointCount)
{
  for (int i = 0; i < pointCount; ++i) {
    drawPoint(points[i]);
  }
}

void ZPainter::drawPoints(const QPoint *points, int pointCount)
{
  for (int i = 0; i < pointCount; ++i) {
    drawPoint(points[i]);
  }
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
  rect.moveCenter(QPointF(m_offset.x(), m_offset.y()) + rectangle.center());
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
