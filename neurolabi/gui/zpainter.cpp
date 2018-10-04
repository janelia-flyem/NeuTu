#include "zpainter.h"

#include <QRect>
#include <QRectF>
#include <QPointF>
#include <QPaintDevice>
#include <QStaticText>

#include "QsLog.h"
#include "neutubeconfig.h"
#include "zintpoint.h"
#include "zimage.h"
#include "tz_math.h"
#include "zpixmap.h"
#include "zrect2d.h"
#include "zviewproj.h"

ZPainter::ZPainter() : m_z(0), m_isPainted(false)
{
}
#ifdef _QT_GUI_USED_
ZPainter::ZPainter(QPaintDevice *device)
{
  m_painter = new QPainter(device);
}

ZPainter::ZPainter(ZImage *image)
{
  begin(image);
}
#endif

ZPainter::~ZPainter()
{
#ifdef _QT_GUI_USED_
  end();

  if (m_painter != NULL) {
    delete m_painter;
  }
#endif
}

#ifdef _QT_GUI_USED_
ZPainter::ZPainter(ZPixmap *pixmap) : m_z(0)
{
  begin(pixmap);
}

QPaintDevice* ZPainter::device()
{
  return getPainter()->device();
}

bool ZPainter::isActive() const
{
  if (m_painter == NULL) {
    return false;
  }

  return getPainter()->isActive();
}

bool ZPainter::begin(ZImage *image)
{
  m_isPainted = false;

  if (getPainter()->begin(image)) {
    QTransform transform;
    const ZStTransform &imageTransform = image->getTransform();
    transform.translate(imageTransform.getTx(), imageTransform.getTy());
    transform.scale(imageTransform.getSx(), imageTransform.getSy());
    getPainter()->setTransform(transform);
    m_transform = imageTransform;
    return true;
  }

  return false;
}

bool ZPainter::restart(ZPixmap *pixmap)
{
  end();
  return begin(pixmap);
}

bool ZPainter::begin(ZPixmap *image)
{
  m_isPainted = false;

  if (getPainter()->begin(image)) {
    updateTransform(image);
    //  qDebug() << this.mapRect(QRectF(100, 100, 200, 200));

    return true;
  }

  return false;
}

void ZPainter::updateTransform(ZPixmap *image)
{
  if (isActive()) {
    QTransform t;
    const ZStTransform &imageTransform = image->getTransform();
    t.translate(imageTransform.getTx(), imageTransform.getTy());
    t.scale(imageTransform.getSx(), imageTransform.getSy());
    m_transform = imageTransform;

    getPainter()->setTransform(t);

    ZOUT(LTRACE(), 5) << t;
    ZOUT(LTRACE(), 5) << this->getTransform();
  }
}

void ZPainter::initPainter()
{
  if (m_painter == NULL) {
    m_painter = new QPainter;
  }
}

QPainter* ZPainter::getPainter()
{
  return const_cast<QPainter*>(static_cast<const ZPainter&>(*this).getPainter());
}

const QPainter* ZPainter::getPainter() const
{
  const_cast<ZPainter&>(*this).initPainter();

  return m_painter;
}

bool ZPainter::begin(QPaintDevice *device)
{
  m_isPainted = false;

  return getPainter()->begin(device);
}

bool ZPainter::end()
{
  m_isPainted = false;

  if (isActive()) {
    return getPainter()->end();
  }

  return true;
}

void ZPainter::attachPainter(QPainter *painter)
{
  m_painter = painter;
}

void ZPainter::detachPainter()
{
  m_painter = NULL;
}

void ZPainter::setPen(const QColor &color)
{
  getPainter()->setPen(color);
}

void ZPainter::setPen(const QPen &pen)
{
  getPainter()->setPen(pen);
}

void ZPainter::setPen(Qt::PenStyle style)
{
  getPainter()->setPen(style);
}

void ZPainter::setFont(const QFont &font)
{
  getPainter()->setFont(font);
}

void ZPainter::setBrush(const QColor &color)
{
  getPainter()->setBrush(color);
}

void ZPainter::setBrush(const QBrush &pen)
{
  getPainter()->setBrush(pen);
}

void ZPainter::setBrush(Qt::BrushStyle style)
{
  getPainter()->setBrush(style);
}

const QPen& ZPainter::getPen() const
{
  return getPainter()->pen();
}

QColor ZPainter::getPenColor() const
{
  return getPen().color();
}

const QBrush& ZPainter::getBrush() const
{
  return getPainter()->brush();
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

QRectF ZPainter::getCanvasRange() const
{
  return m_canvasRange;
}

void ZPainter::drawImage(
    const QRectF &targetRect, const ZImage &image, const QRectF &sourceRect)
{
  if (!image.isNull()) {
    getPainter()->drawImage(
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
//    x = iround(image.getTransform().transformX(x));
//    y = iround(image.getTransform().transformY(y));
    QRect targetRect = QRect(
          x, y, iround(image.width() / image.getTransform().getSx()),
          iround(image.height() / image.getTransform().getSy()));
    QRect sourceRect = QRect(0, 0, image.width(), image.height());
    getPainter()->drawImage(targetRect,
                        dynamic_cast<const QImage&>(image), sourceRect);
    setPainted(true);
  }
}

void ZPainter::drawImage(const ZViewProj &viewProj, const ZImage &image)
{
  if (viewProj.isSourceValid()) {
    drawImage(viewProj.getProjRect(), image, viewProj.getViewPort());
  }
}

void ZPainter::drawPixmap(
    const QRectF &targetRect, const ZPixmap &image, const QRectF &sourceRect)
{
  if (sourceRect.isValid() && targetRect.isValid() && !image.isNull()) {
    //Transform from world coordinates to image coordinates
    QRectF newSourceRect = image.getTransform().transform(sourceRect);

    getPainter()->drawPixmap(targetRect, image, newSourceRect);

    setPainted(true);
  }
}

void ZPainter::drawPixmap(const ZViewProj &viewProj, const ZPixmap &image)
{
  drawPixmap(viewProj.getProjRect(), image, viewProj.getViewPort());
}

void ZPainter::drawPixmap(const QRectF &targetRect, const ZPixmap &image)
{
  if (targetRect.isValid() && !image.isNull()) {
    getPainter()->drawPixmap(targetRect, image, image.rect());

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
        newSourceRect.intersected(image.getActiveArea(neutube::ECoordinateSystem::WORLD_2D));
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

    getPainter()->drawPixmap(
          targetRect, dynamic_cast<const QPixmap&>(image), sourceRect);

    setPainted(true);
  }
}

void ZPainter::drawPixmapNt(const ZPixmap &image)
{
  if (!image.isNull()) {
    getPainter()->drawPixmap(0, 0, image);
    setPainted(true);
  }
}

void ZPainter::drawPixmap(const ZPixmap &image)
{
  if (!image.isNull()) {
    QRectF targetRect =
        image.getProjTransform().transform(QRectF(image.rect()));
    getPainter()->drawPixmap(targetRect, image, image.rect());
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
          sourceRect.intersected(image.getActiveArea(neutube::ECoordinateSystem::WORLD_2D));
      if (!sourceRect.isEmpty()) {
        targetRect = ZRect2d::CropRect(oldSourceRect, sourceRect, targetRect);
      }
    }

    if (sourceRect.isValid()) {
      getPainter()->drawPixmap(
            targetRect, dynamic_cast<const QPixmap&>(image), sourceRect);

      setPainted(true);
    }
  }
}

const QTransform& ZPainter::getTransform() const
{
  return getPainter()->transform();
}

void ZPainter::setTransform(const QTransform &t, bool combine)
{
  getPainter()->setTransform(t, combine);
}


void ZPainter::drawPoint(const QPointF &pt)
{
  getPainter()->drawPoint(pt);
  setPainted(true);

//  QPainter::drawPoint(m_transform.transform(pt));
//  QPainter::drawPoint(pt - QPointF(m_offset.x(), m_offset.y()));
}


void ZPainter::drawText(
    int x, int y, int width, int height, int flags, const QString &text)
{
  if (isVisible(QRect(QPoint(x, y), QSize(width, height)))) {
    getPainter()->drawText(x, y, width, height, flags, text);
    setPainted(true);
  }
}

void ZPainter::drawStaticText(int x, int y, const QStaticText &text)
{
  if (isVisible(QRect(QPoint(x, y), text.size().toSize()))) {
    getPainter()->drawStaticText(x, y, text);
    setPainted(true);
  }
}

void ZPainter::drawPoint(const QPoint &pt)
{
  getPainter()->drawPoint(pt);
  setPainted(true);

//  QPainter::drawPoint(m_transform.transform(pt));
//  QPainter::drawPoint(pt - QPointF(m_offset.x(), m_offset.y()));
}

void ZPainter::drawPoints(const QPointF *points, int pointCount)
{
  if (pointCount > 0 && points != NULL) {
    getPainter()->drawPoints(points, pointCount);
    setPainted(true);
  }
//  for (int i = 0; i < pointCount; ++i) {
//    drawPoint(points[i]);
//  }
}


void ZPainter::drawPoints(const QPoint *points, int pointCount)
{
  if (pointCount > 0 && points != NULL) {
    getPainter()->drawPoints(points, pointCount);
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
  if (isVisible(x1, y1, x2, y2)) {
    getPainter()->drawLine(x1, y1, x2, y2);
    setPainted(true);
  }
//  drawLine(QPointF(x1, y1), QPointF(x2, y2));
}

bool ZPainter::isVisible(double x1, double y1, double x2, double y2) const
{
  if (m_canvasRange.isEmpty()) {
    return true;
  }

  if (x1 > x2) {
    std::swap(x1, x2);
  }

  if (y1 > y2) {
    std::swap(y1, y2);
  }

  QRectF rect;
  rect.setTopLeft(QPointF(x1 - 0.5, y1 - 0.5));
  rect.setBottomRight(QPointF(x2 + 0.5, y2 + 0.5));
  bool visible = m_canvasRange.intersects(rect);

  return visible;
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
    getPainter()->drawLine(pt1, pt2);
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
  getPainter()->drawLines(lines, lineCount);
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
    getPainter()->drawEllipse(rectangle);
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
    getPainter()->drawEllipse(rectangle);
    setPainted(true);
  }
//  drawEllipse(QRectF(rectangle));
}

void ZPainter::drawEllipse(int x, int y, int width, int height)
{
  if (isVisible(QRectF(x, y, width, height))) {
    getPainter()->drawEllipse(x, y, width, height);
    setPainted(true);
  }

//  drawEllipse(QPointF(x, y), width, height);
}

void ZPainter::drawEllipse(const QPointF & center, double rx, double ry)
{
  if (isVisible(QRectF(center.x() - rx, center.y() - ry,
                       rx + rx, ry + ry))) {
    getPainter()->drawEllipse(center, rx, ry);
    setPainted(true);
  }

#if _QT_GUI_USED_
//  QPointF newCenter = center - QPointF(m_offset.x(), m_offset.y());
//  QPainter::drawEllipse(m_transform.transform(center),
//                        rx * m_transform.getSx(),
//                        ry * m_transform.getSy());
#endif
}

void ZPainter::drawArc(const QRectF &rectangle, int startAngle, int spanAngle)
{
  if (isVisible(rectangle)) {
    getPainter()->drawArc(rectangle, startAngle, spanAngle);
    setPainted(true);
  }
}

void ZPainter::drawCross(const QPointF &center, double radius)
{
  drawLine(QPointF(center.x(), center.y() + radius),
                   QPointF(center.x() + radius, center.y()));
  drawLine(QPointF(center.x(), center.y() + radius),
                   QPointF(center.x() - radius, center.y()));
}

void ZPainter::drawEllipse(const QPoint & center, int rx, int ry)
{
  if (isVisible(QRectF(center.x() - rx, center.y() - ry,
                       rx + rx, ry + ry))) {
    getPainter()->drawEllipse(center, rx, ry);
    setPainted(true);
  }

  //  drawEllipse(QPointF(center), rx, ry);
}

void ZPainter::drawRect(const QRectF & rectangle)
{
  if (isVisible(rectangle)) {
    getPainter()->drawRect(rectangle);
    setPainted(true);
  }
#if _QT_GUI_USED_
//  QRectF rect = rectangle;
//  rect.moveCenter(-QPointF(m_offset.x(), m_offset.y()) + rectangle.center());
//  QPainter::drawRect(m_transform.transform(rect));
#endif
}

void ZPainter::drawRect(const QRect & rectangle)
{
  if (isVisible(rectangle)) {
    getPainter()->drawRect(rectangle);
    setPainted(true);
  }
//  drawRect(QRectF(rectangle));
}

void ZPainter::drawRect(int x, int y, int width, int height)
{
  if (isVisible(QRect(QPoint(x, y), QSize(width, height)))) {
    getPainter()->drawRect(x, y, width, height);
    setPainted(true);
  }
//  drawRect(QRectF(x, y, width, height));
}

void ZPainter::drawPolyline(const QPointF * points, int pointCount)
{
  if (points != NULL && pointCount > 0) {
    double x1 = points[0].x();
    double y1 = points[0].y();
    double x2 = x1;
    double y2 = y1;

    for (int i = 1; i < pointCount; ++i) {
      const QPointF &pt = points[i];
      if (x1 > pt.x()) {
        x1 = pt.x();
      } else if (x2 < pt.x()) {
        x2 = pt.x();
      }

      if (y1 > pt.y()) {
        y1 = pt.y();
      } else if (y2 < pt.y()) {
        y2 = pt.y();
      }
    }

    if (isVisible(x1, y1, x2, y2)) {
      getPainter()->drawPolyline(points, pointCount);
      setPainted(true);
    }
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
    getPainter()->drawPolyline(points, pointCount);
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
  getPainter()->save();
}

void ZPainter::restore()
{
  getPainter()->restore();
}

void ZPainter::setCompositionMode(QPainter::CompositionMode mode)
{
  getPainter()->setCompositionMode(mode);
}

void ZPainter::setRenderHints(QPainter::RenderHints hints, bool on)
{
  getPainter()->setRenderHints(hints, on);
}

void ZPainter::setRenderHint(QPainter::RenderHint hint, bool on)
{
  getPainter()->setRenderHint(hint, on);
}

void ZPainter::fillRect(const QRect &r, Qt::GlobalColor color)
{
  getPainter()->fillRect(r, color);
}

void ZPainter::setOpacity(double alpha)
{
  getPainter()->setOpacity(alpha);
}
#endif
