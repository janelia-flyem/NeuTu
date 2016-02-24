#include "zpainter.h"

#include <QRect>
#include <QRectF>
#include <QPointF>
#include <QPaintDevice>

#include "zintpoint.h"
#include "zimage.h"
#include "tz_math.h"
#include "zpixmap.h"
#include "zrect2d.h"

ZPainter::ZPainter() : m_z(0), m_isPainted(false)
{

}
#ifdef _QT_GUI_USED_
ZPainter::ZPainter(QPaintDevice *device) :
  m_painter(device),
  m_z(0), m_isPainted(false)
{
}

ZPainter::ZPainter(ZImage *image) : m_z(0)
{
  begin(image);
  /*
  QTransform transform;
  const ZStTransform &imageTransform = image->getTransform();
  transform.scale(imageTransform.getSx(), imageTransform.getSy());
  transform.translate(imageTransform.getTx(), imageTransform.getTy());
  setTransform(transform);
  */
}
#endif

ZPainter::~ZPainter()
{
#ifdef _QT_GUI_USED_
  end();
#endif
}

#ifdef _QT_GUI_USED_
ZPainter::ZPainter(ZPixmap *pixmap) : m_z(0)
{
  begin(pixmap);
}

QPaintDevice* ZPainter::device()
{
  return m_painter.device();
}

bool ZPainter::isActive() const
{
  return m_painter.isActive();
}

bool ZPainter::begin(ZImage *image)
{
  m_isPainted = false;

  if (m_painter.begin(image)) {
    QTransform transform;
    const ZStTransform &imageTransform = image->getTransform();
    transform.translate(imageTransform.getTx(), imageTransform.getTy());
    transform.scale(imageTransform.getSx(), imageTransform.getSy());
    m_painter.setTransform(transform);
    return true;
  }

  return false;
}

bool ZPainter::begin(ZPixmap *image)
{
  m_isPainted = false;

  if (m_painter.begin(image)) {
    QTransform t;
    const ZStTransform &imageTransform = image->getTransform();
    t.translate(imageTransform.getTx(), imageTransform.getTy());
    t.scale(imageTransform.getSx(), imageTransform.getSy());

    m_painter.setTransform(t);

#ifdef _DEBUG_2
    qDebug() << t;
    qDebug() << this->getTransform();
    //  qDebug() << this.mapRect(QRectF(100, 100, 200, 200));
#endif
    return true;
  }

  return false;
}

bool ZPainter::begin(QPaintDevice *device)
{
  m_isPainted = false;

  return m_painter.begin(device);
}

bool ZPainter::end()
{
  m_isPainted = false;

  if (m_painter.isActive()) {
    return m_painter.end();
  }

  return true;
}

void ZPainter::setPen(const QColor &color)
{
  m_painter.setPen(color);
}

void ZPainter::setPen(const QPen &pen)
{
  m_painter.setPen(pen);
}

void ZPainter::setPen(Qt::PenStyle style)
{
  m_painter.setPen(style);
}

void ZPainter::setBrush(const QColor &color)
{
  m_painter.setBrush(color);
}

void ZPainter::setBrush(const QBrush &pen)
{
  m_painter.setBrush(pen);
}

void ZPainter::setBrush(Qt::BrushStyle style)
{
  m_painter.setBrush(style);
}

const QPen& ZPainter::getPen() const
{
  return m_painter.pen();
}

QColor ZPainter::getPenColor() const
{
  return getPen().color();
}

const QBrush& ZPainter::getBrush() const
{
  return m_painter.brush();
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
  if (!image.isNull()) {
    m_painter.drawImage(
          targetRect, image, image.getTransform().transform(sourceRect));
    setPainted(true);
  }
}

void ZPainter::drawImage(int x, int y, const ZImage &image)
{
  if (!image.isNull()) {
    //qDebug() << getTransform();

    //  QRect targetRect = transform().mapRect(QRect(
    //        x, y, iround(image.width() / image.getTransform().getSx()),
    //        iround(image.height() / image.getTransform().getSy())));
    QRect targetRect = QRect(
          x, y, iround(image.width() / image.getTransform().getSx()),
          iround(image.height() / image.getTransform().getSy()));
    QRect sourceRect = QRect(0, 0, image.width(), image.height());
    m_painter.drawImage(targetRect,
                        dynamic_cast<const QImage&>(image), sourceRect);
    setPainted(true);
  }
}

void ZPainter::drawPixmap(
    const QRectF &targetRect, const ZPixmap &image, const QRectF &sourceRect)
{
  if (sourceRect.isValid() && targetRect.isValid() && !image.isNull()) {
    //Transform from world coordinates to image coordinates
    QRectF newSourceRect = image.getTransform().transform(sourceRect);

    m_painter.drawPixmap(targetRect, image, newSourceRect);

    setPainted(true);
  }
}

void ZPainter::drawActivePixmap(
    const QRectF &targetRect, const ZPixmap &image, const QRectF &sourceRect)
{
  QRectF newSourceRect = sourceRect;
  QRectF newTargetRect = targetRect;

  if (!image.isFullyActive()) {
    newSourceRect =
        newSourceRect.intersected(image.getActiveArea(NeuTube::COORD_WORLD));
    if (!newSourceRect.isEmpty()) {
      newTargetRect = ZRect2d::CropRect(sourceRect, newSourceRect, targetRect);
    }
  }

  if (!newSourceRect.isEmpty()) {
    drawPixmap(newTargetRect, image, newSourceRect);
  }
}

void ZPainter::drawPixmap(int x, int y, const ZPixmap &image)
{
  if (!image.isNull()) {
    QRect sourceRect = QRect(0, 0, image.width(), image.height());

    QRectF targetRect = QRectF(
          x, y, iround(sourceRect.width() / image.getTransform().getSx()),
          iround(sourceRect.height() / image.getTransform().getSy()));

    m_painter.drawPixmap(
          targetRect, dynamic_cast<const QPixmap&>(image), sourceRect);

    setPainted(true);
  }
}

void ZPainter::drawActivePixmap(int x, int y, const ZPixmap &image)
{
  if (!image.isNull()) {
    QRectF sourceRect = QRect(0, 0, image.width(), image.height());

    QRectF targetRect = QRectF(
          x, y, iround(sourceRect.width() / image.getTransform().getSx()),
          iround(sourceRect.height() / image.getTransform().getSy()));

    if (!image.isFullyActive()) {
      QRectF oldSourceRect = sourceRect;
      sourceRect =
          sourceRect.intersected(image.getActiveArea(NeuTube::COORD_WORLD));
      if (!sourceRect.isEmpty()) {
        targetRect = ZRect2d::CropRect(oldSourceRect, sourceRect, targetRect);
      }
    }

    if (sourceRect.isValid()) {
      m_painter.drawPixmap(
            targetRect, dynamic_cast<const QPixmap&>(image), sourceRect);

      setPainted(true);
    }
  }
}

const QTransform& ZPainter::getTransform() const
{
  return m_painter.transform();
}

void ZPainter::setTransform(const QTransform &t, bool combine)
{
  m_painter.setTransform(t, combine);
}


void ZPainter::drawPoint(const QPointF &pt)
{
  m_painter.drawPoint(pt);
  setPainted(true);

//  QPainter::drawPoint(m_transform.transform(pt));
//  QPainter::drawPoint(pt - QPointF(m_offset.x(), m_offset.y()));
}


void ZPainter::drawText(
    int x, int y, int width, int height, int flags, const QString &text)
{
  m_painter.drawText(x, y, width, height, flags, text);
  setPainted(true);
}

void ZPainter::drawPoint(const QPoint &pt)
{
  m_painter.drawPoint(pt);
  setPainted(true);

//  QPainter::drawPoint(m_transform.transform(pt));
//  QPainter::drawPoint(pt - QPointF(m_offset.x(), m_offset.y()));
}

void ZPainter::drawPoints(const QPointF *points, int pointCount)
{
  if (pointCount > 0 && points != NULL) {
    m_painter.drawPoints(points, pointCount);
    setPainted(true);
  }
//  for (int i = 0; i < pointCount; ++i) {
//    drawPoint(points[i]);
//  }
}

void ZPainter::drawPoints(const QPoint *points, int pointCount)
{
  if (pointCount > 0 && points != NULL) {
    m_painter.drawPoints(points, pointCount);
    setPainted(true);
  }
//  for (int i = 0; i < pointCount; ++i) {
//    drawPoint(points[i]);
//  }
}

void ZPainter::drawPoints(const std::vector<QPoint> &pointArray)
{
  drawPoints(&(pointArray[0]), pointArray.size());
}

void ZPainter::drawPoints(const std::vector<QPointF> &pointArray)
{
  drawPoints(&(pointArray[0]), pointArray.size());
}

void ZPainter::drawLine(int x1, int y1, int x2, int y2)
{
  if (isVisible(QRectF(x1, y1, x2, y2))) {
    m_painter.drawLine(x1, y1, x2, y2);
    setPainted(true);
  }
//  drawLine(QPointF(x1, y1), QPointF(x2, y2));
}

bool ZPainter::isVisible(const QRect &rect) const
{
  return isVisible(QRectF(rect));
}

bool ZPainter::isVisible(const QRectF &rect) const
{
  if (rect.isEmpty()) {
    return false;
  }

  if (m_canvasRange.isEmpty()) {
    return true;
  }

  QRectF calrRect = rect.normalized();
  calrRect.setLeft(calrRect.left() - 0.5);
  calrRect.setRight(calrRect.right() + 0.5);
  calrRect.setTop(calrRect.top() - 0.5);
  calrRect.setBottom(calrRect.bottom() + 0.5);

  bool visible = m_canvasRange.intersects(calrRect);

  return visible;
}

void ZPainter::drawLine(const QPointF &pt1, const QPointF &pt2)
{
  QRectF rect(std::min(pt1.x(), pt2.x()), std::min(pt1.y(), pt2.y()),
              fabs(pt1.x() - pt2.x()) + 1.0, fabs(pt1.y() - pt2.y()) + 1.0);

  if (isVisible(rect)) {
    m_painter.drawLine(pt1, pt2);
    setPainted(true);
  }

//#if _QT_GUI_USED_
//  QPainter::drawLine(pt1 - QPointF(m_offset.x(), m_offset.y()),
//                     pt2 - QPointF(m_offset.x(), m_offset.y()));

//  QPainter::drawLine(m_transform.transform(pt1),
//                     m_transform.transform(pt2));
//#endif
}

void ZPainter::drawLines(const QLine *lines, int lineCount)
{
  m_painter.drawLines(lines, lineCount);
  setPainted(true);
}

void ZPainter::drawLines(const std::vector<QLine> &lineArray)
{
  if (!lineArray.empty()) {
    drawLines(&(lineArray[0]), lineArray.size());
  }
}

void ZPainter::drawEllipse(const QRectF & rectangle)
{
  if (isVisible(rectangle)) {
    m_painter.drawEllipse(rectangle);
    setPainted(true);
  }
//#if _QT_GUI_USED_
//  QRectF rect = rectangle;
//  rect.moveCenter(-QPointF(m_offset.x(), m_offset.y()));
//  QPainter::drawEllipse(m_transform.transform(rectangle));
//#endif
}

void ZPainter::drawEllipse(const QRect & rectangle)
{
  if (isVisible(QRectF(rectangle))) {
    m_painter.drawEllipse(rectangle);
    setPainted(true);
  }
//  drawEllipse(QRectF(rectangle));
}

void ZPainter::drawEllipse(int x, int y, int width, int height)
{
  if (isVisible(QRectF(x, y, width, height))) {
    m_painter.drawEllipse(x, y, width, height);
    setPainted(true);
  }

//  drawEllipse(QPointF(x, y), width, height);
}

void ZPainter::drawEllipse(const QPointF & center, double rx, double ry)
{
  if (isVisible(QRectF(center.x() - rx, center.y() - ry,
                       rx + rx, ry + ry))) {
    m_painter.drawEllipse(center, rx, ry);
    setPainted(true);
  }

#if _QT_GUI_USED_
//  QPointF newCenter = center - QPointF(m_offset.x(), m_offset.y());
//  QPainter::drawEllipse(m_transform.transform(center),
//                        rx * m_transform.getSx(),
//                        ry * m_transform.getSy());
#endif
}

void ZPainter::drawEllipse(const QPoint & center, int rx, int ry)
{
  if (isVisible(QRectF(center.x() - rx, center.y() - ry,
                       rx + rx, ry + ry))) {
    m_painter.drawEllipse(center, rx, ry);
    setPainted(true);
  }

  //  drawEllipse(QPointF(center), rx, ry);
}

void ZPainter::drawRect(const QRectF & rectangle)
{
  m_painter.drawRect(rectangle);
  setPainted(true);
#if _QT_GUI_USED_
//  QRectF rect = rectangle;
//  rect.moveCenter(-QPointF(m_offset.x(), m_offset.y()) + rectangle.center());
//  QPainter::drawRect(m_transform.transform(rect));
#endif
}

void ZPainter::drawRect(const QRect & rectangle)
{
  m_painter.drawRect(rectangle);
  setPainted(true);
//  drawRect(QRectF(rectangle));
}

void ZPainter::drawRect(int x, int y, int width, int height)
{
  m_painter.drawRect(x, y, width, height);
  setPainted(true);
//  drawRect(QRectF(x, y, width, height));
}

void ZPainter::drawPolyline(const QPointF * points, int pointCount)
{
  if (points != NULL && pointCount > 0) {
    m_painter.drawPolyline(points, pointCount);
    setPainted(true);
  }

#ifdef _QT_GUI_USED_
//  QPointF *newPoints = new QPointF[pointCount];
//  for (int i = 0; i < pointCount; ++i) {
//    newPoints[i] = m_transform.transform(points[i]);
//    /*
//    newPoints[i].setX(points[i].x() - m_offset.x());
//    newPoints[i].setY(points[i].y() - m_offset.y());*/
//  }
//  QPainter::drawPolyline(newPoints, pointCount);
//  delete []newPoints;
#endif
}

void ZPainter::drawPolyline(const QPoint * points, int pointCount)
{
  if (points != NULL && pointCount > 0) {
    m_painter.drawPolyline(points, pointCount);
    setPainted(true);
  }

#ifdef _QT_GUI_USED_
//  QPointF *newPoints = new QPointF[pointCount];
//  for (int i = 0; i < pointCount; ++i) {
//    newPoints[i] = m_transform.transform(points[i]);
////    newPoints[i].setX(points[i].x() - m_offset.x());
////    newPoints[i].setY(points[i].y() - m_offset.y());
//  }
//  QPainter::drawPolyline(newPoints, pointCount);
//  delete []newPoints;
#endif
}

void ZPainter::save()
{
  m_painter.save();
}

void ZPainter::restore()
{
  m_painter.restore();
}

void ZPainter::setCompositionMode(QPainter::CompositionMode mode)
{
  m_painter.setCompositionMode(mode);
}

void ZPainter::setRenderHints(QPainter::RenderHints hints, bool on)
{
  m_painter.setRenderHints(hints, on);
}

void ZPainter::setRenderHint(QPainter::RenderHint hint, bool on)
{
  m_painter.setRenderHint(hint, on);
}

void ZPainter::fillRect(const QRect &r, Qt::GlobalColor color)
{
  m_painter.fillRect(r, color);
}

void ZPainter::setOpacity(double alpha)
{
  m_painter.setOpacity(alpha);
}
#endif
