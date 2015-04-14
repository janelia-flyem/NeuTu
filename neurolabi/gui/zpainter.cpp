#include "zpainter.h"
#include "zintpoint.h"
#include "zimage.h"
#include "tz_math.h"
#include "zpixmap.h"

ZPainter::ZPainter() : m_z(0)
{

}

ZPainter::ZPainter(QPaintDevice *device) : QPainter(device), m_z(0)
{
}

ZPainter::ZPainter(ZImage *image) : QPainter(image), m_z(0)
{
  QTransform transform;
  const ZStTransform &imageTransform = image->getTransform();
  transform.scale(imageTransform.getSx(), imageTransform.getSy());
  transform.translate(imageTransform.getTx(), imageTransform.getTy());
  setTransform(transform);
}

bool ZPainter::begin(ZImage *image)
{
  if (QPainter::begin(image)) {
    QTransform transform;
    const ZStTransform &imageTransform = image->getTransform();
    transform.scale(imageTransform.getSx(), imageTransform.getSy());
    transform.translate(imageTransform.getTx(), imageTransform.getTy());
    setTransform(transform);
    return true;
  }

  return false;
}

bool ZPainter::begin(ZPixmap *image)
{
  if (QPainter::begin(image)) {
    QTransform t;
    const ZStTransform &imageTransform = image->getTransform();
    t.scale(imageTransform.getSx(), imageTransform.getSy());
    t.translate(imageTransform.getTx(), imageTransform.getTy());
    setTransform(t);

#ifdef _DEBUG_
    qDebug() << t;
    qDebug() << this->transform();
    //  qDebug() << this.mapRect(QRectF(100, 100, 200, 200));
#endif
    return true;
  }

  return false;
}

bool ZPainter::begin(QPaintDevice *device)
{
  return QPainter::begin(device);
}


void ZPainter::setStackOffset(int x, int y, int z)
{
  QTransform transform;
  transform.translate(-x, -y);
  setTransform(transform);
  m_z = z;
//  m_offset.set(x, y, z);
  //m_transform.setTranslate(-x, -y, -z);
}

void ZPainter::setStackOffset(const ZIntPoint &offset)
{
  setStackOffset(offset.getX(), offset.getY(), offset.getZ());
}

void ZPainter::setStackOffset(const ZPoint &offset)
{
  setStackOffset(offset.x(), offset.y(), offset.z());
  //m_offset.set(offset.x(), offset.y(), offset.z());
}

void ZPainter::setZOffset(int z)
{
  m_z = z;
}

void ZPainter::drawImage(
    const QRectF &targetRect, const ZImage &image, const QRectF &sourceRect)
{
  QPainter::drawImage(
        targetRect, image, image.getTransform().transform(sourceRect));
}

void ZPainter::drawImage(int x, int y, const ZImage &image)
{
  qDebug() << transform();

//  QRect targetRect = transform().mapRect(QRect(
//        x, y, iround(image.width() / image.getTransform().getSx()),
//        iround(image.height() / image.getTransform().getSy())));
  QRect targetRect = QRect(
          x, y, iround(image.width() / image.getTransform().getSx()),
          iround(image.height() / image.getTransform().getSy()));
  QRect sourceRect = QRect(0, 0, image.width(), image.height());
  QPainter::drawImage(targetRect,
                      dynamic_cast<const QImage&>(image), sourceRect);
}

void ZPainter::drawPixmap(
    const QRectF &targetRect, const ZPixmap &image, const QRectF &sourceRect)
{
  QRectF newSourceRect = image.getTransform().transform(sourceRect);
  QPainter::drawPixmap(targetRect, image, newSourceRect);
}

void ZPainter::drawPixmap(int x, int y, const ZPixmap &image)
{
  QRect targetRect = QRect(
        x, y, iround(image.width() / image.getTransform().getSx()),
        iround(image.height() / image.getTransform().getSy()));
  QRect sourceRect = QRect(0, 0, image.width(), image.height());
  QPainter::drawPixmap(
        targetRect, dynamic_cast<const QPixmap&>(image), sourceRect);
}

#if 0
void ZPainter::drawPoint(const QPointF &pt)
{
  QPainter::drawPoint(m_transform.transform(pt));
//  QPainter::drawPoint(pt - QPointF(m_offset.x(), m_offset.y()));
}

void ZPainter::drawPoint(const QPoint &pt)
{
  QPainter::drawPoint(m_transform.transform(pt));
//  QPainter::drawPoint(pt - QPointF(m_offset.x(), m_offset.y()));
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
//  QPainter::drawLine(pt1 - QPointF(m_offset.x(), m_offset.y()),
//                     pt2 - QPointF(m_offset.x(), m_offset.y()));

  QPainter::drawLine(m_transform.transform(pt1),
                     m_transform.transform(pt2));
#endif
}

void ZPainter::drawEllipse(const QRectF & rectangle)
{
#if _QT_GUI_USED_
//  QRectF rect = rectangle;
//  rect.moveCenter(-QPointF(m_offset.x(), m_offset.y()));
  QPainter::drawEllipse(m_transform.transform(rectangle));
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

void ZPainter::drawEllipse(const QPointF & center, double rx, double ry)
{
#if _QT_GUI_USED_
//  QPointF newCenter = center - QPointF(m_offset.x(), m_offset.y());
  QPainter::drawEllipse(m_transform.transform(center),
                        rx * m_transform.getSx(),
                        ry * m_transform.getSy());
#endif
}

void ZPainter::drawEllipse(const QPoint & center, int rx, int ry)
{
  drawEllipse(QPointF(center), rx, ry);
}

void ZPainter::drawRect(const QRectF & rectangle)
{
#if _QT_GUI_USED_
//  QRectF rect = rectangle;
//  rect.moveCenter(-QPointF(m_offset.x(), m_offset.y()) + rectangle.center());
  QPainter::drawRect(m_transform.transform(rect));
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
    newPoints[i] = m_transform.transform(points[i]);
    /*
    newPoints[i].setX(points[i].x() - m_offset.x());
    newPoints[i].setY(points[i].y() - m_offset.y());*/
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
    newPoints[i] = m_transform.transform(points[i]);
//    newPoints[i].setX(points[i].x() - m_offset.x());
//    newPoints[i].setY(points[i].y() - m_offset.y());
  }
  QPainter::drawPolyline(newPoints, pointCount);
  delete []newPoints;
#endif
}
#endif
