#include "zstackball.h"

#include "zqtheader.h"

#include <QRectF>
#include <QPen>

#include <math.h>
#include "tz_math.h"
#include "zintpoint.h"
#include "zpainter.h"

ZStackBall::ZStackBall()
{
  init(0, 0, 0, 1);
}

ZStackBall::ZStackBall(double x, double y, double z, double r)
{
  init(x, y, z, r);
}

ZStackBall::ZStackBall(const ZIntPoint &center, double r)
{
  init(center.getX(), center.getY(), center.getZ(), r);
}

void ZStackBall::init(double x, double y, double z, double r)
{
  set(x, y, z, r);
  m_type = ZStackObject::TYPE_STACK_BALL;
}


void ZStackBall::set(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_r = r;
}

void ZStackBall::set(const ZIntPoint &center, double r)
{
  set(center.getX(), center.getY(), center.getZ(), r);
}

void ZStackBall::set(const ZPoint &center, double r)
{
  set(center.x(), center.y(), center.z(), r);
}

void ZStackBall::setCenter(double x, double y, double z)
{
  m_center.set(x, y, z);
}

void ZStackBall::setCenter(const ZPoint &center)
{
  m_center = center;
}

void ZStackBall::setCenter(const ZIntPoint &center)
{
  setCenter(center.getX(), center.getY(), center.getZ());
}

//void ZCircle::display(QImage *image, int n, Display_Style style) const
//{
//#if defined(_QT_GUI_USED_)
//  QPainter painter(image);
//  painter.setPen(m_color);
//  display(&painter, n, style);
//#endif
//}

void ZStackBall::display(ZPainter &painter, int slice,
                         ZStackObject::EDisplayStyle style,
                         NeuTube::EAxis sliceAxis) const
{
  if (!isVisible()) {
    return;
  }

//  UNUSED_PARAMETER(style);
#if _QT_GUI_USED_
  ZPoint shiftedCenter = getCenter();
  shiftedCenter.shiftSliceAxis(sliceAxis);

  if (!painter.isVisible(QRectF(shiftedCenter.x() - getRadius(),
                                shiftedCenter.y() - getRadius(),
                                getRadius() * 2,
                                getRadius() * 2))) {
    return;
  }


//  painter.save();

//  const QPen &oldPen = painter.pen();
//  const QBrush &oldBrush = painter.brush();

  QPen pen(m_color, getPenWidth());
  pen.setCosmetic(m_usingCosmeticPen);

  if (hasVisualEffect(NeuTube::Display::Sphere::VE_DASH_PATTERN)) {
    pen.setStyle(Qt::DotLine);
  }

  painter.setPen(pen);

#ifdef _DEBUG_
  if (m_color.red() < 64 && m_color.green() <64 && m_color.blue() < 64) {
    qDebug() << "Internal color: " << m_color;
  }
#endif

  if (hasVisualEffect(NeuTube::Display::Sphere::VE_GRADIENT_FILL)) {
    QRadialGradient gradient(50, 50, 50, 50, 50);
    gradient.setColorAt(0, QColor::fromRgbF(0, 1, 0, 1));
    gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));

    /*
    QBrush brush(gradient);
    brush.setColor(m_color);
    brush.setStyle(Qt::RadialGradientPattern);
    painter.setBrush(brush);
    */
    painter.setBrush(m_color);
    painter.setBrush(QBrush(m_color, Qt::RadialGradientPattern));
  } else {
    if (hasVisualEffect(NeuTube::Display::Sphere::VE_NO_FILL)) {
      painter.setBrush(Qt::NoBrush);
    }
  }
  displayHelper(&painter, slice, style, sliceAxis);

  m_prevDisplaySlice = slice;

//  painter.setPen(oldPen);
//  painter.setBrush(oldBrush);

//  painter.restore();
#endif
}

bool ZStackBall::isCuttingPlane(double z, double r, double n, double zScale)
{
  double h = fabs(z - n) / zScale;
  if (r > h) {
    return true;
  } else if (iround(z) == iround(n)) {
    return true;
  }

  return false;
}

bool ZStackBall::isCuttingPlane(
    double n, double zScale, NeuTube::EAxis sliceAxis) const
{
  double z = m_center.getSliceCoord(sliceAxis);

  return isCuttingPlane(z, m_r, n, zScale);
}

bool ZStackBall::isSliceVisible(int z, NeuTube::EAxis sliceAxis) const
{
  if (isVisible()) {
    if (isCuttingPlane(z, m_zScale, sliceAxis) || isSelected()) {
      return true;
    }
  }

  return false;
}

double ZStackBall::getAdjustedRadius(double r) const
{
  double adjustedRadius = r;
  if (!m_usingCosmeticPen) {
    adjustedRadius += getPenWidth() * 0.5;
  }

  return adjustedRadius;
}

void ZStackBall::displayHelper(
    ZPainter *painter, int slice, EDisplayStyle style,
    NeuTube::EAxis sliceAxis) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  double adjustedRadius = getAdjustedRadius(m_r);

  ZPoint shiftedCenter = getCenter();
  shiftedCenter.shiftSliceAxis(sliceAxis);

  double dataFocus = slice + painter->getZOffset();
  bool visible = false;

//  const QBrush &oldBrush = painter->brush();
  const QPen &oldPen = painter->getPen();

  QPen pen;
  pen.setWidthF(getPenWidth());
  pen.setCosmetic(m_usingCosmeticPen);
  if (hasVisualEffect(NeuTube::Display::Sphere::VE_DASH_PATTERN)) {
    pen.setStyle(Qt::DotLine);
  }
  double alpha = oldPen.color().alphaF();

  bool isFocused = false;

  if (slice < 0) {
    visible = true;
  } else {
    if (isCuttingPlane(shiftedCenter.z(), m_r, dataFocus, m_zScale)) {
      double dz = fabs(shiftedCenter.z() - dataFocus);
      if (dz < 0.5) {
        isFocused = true;
      }
      double h = dz / m_zScale;
      double r = 0.0;
      if (m_r > h) {
        r = sqrt(m_r * m_r - h * h);
        adjustedRadius = getAdjustedRadius(r);
        //adjustedRadius = r + getPenWidth() * 0.5;
        visible = true;
      } else { //too small, show at least one plane
        //adjustedRadius = getPenWidth() * 0.5;
        r = 0.1;
        adjustedRadius = getAdjustedRadius(r);
        visible = true;
      }
      if (hasVisualEffect(NeuTube::Display::Sphere::VE_OUT_FOCUS_DIM)) {
        if (!isFocused) {
          alpha *= r * r / m_r / m_r * 0.5 + 0.1;
        }
        //alpha *= alpha;
      }
    }
  }

  if (visible) {
    QColor color = painter->getPenColor();
    color.setAlphaF(alpha);
    pen.setColor(color);
    painter->setPen(pen);

    if ((!hasVisualEffect(NeuTube::Display::Sphere::VE_NO_FILL)) &&
        hasVisualEffect(NeuTube::Display::Sphere::VE_FORCE_FILL)) {
      QBrush brush(color);
      painter->setBrush(brush);
    }

    if (!hasVisualEffect(NeuTube::Display::Sphere::VE_NO_CIRCLE) &&
        !hasVisualEffect(NeuTube::Display::Sphere::VE_RECTANGLE_SHAPE)) {
      //qDebug() << painter->brush().color();
      painter->drawEllipse(QPointF(shiftedCenter.x(), shiftedCenter.y()),
                           adjustedRadius, adjustedRadius);
    } else if (hasVisualEffect(NeuTube::Display::Sphere::VE_RECTANGLE_SHAPE)) {
      double rectWidth = adjustedRadius * 2.0;
      painter->drawRect(
            QRectF(QPointF(shiftedCenter.x(), shiftedCenter.y()),
                   QSizeF(rectWidth, rectWidth)));
    }

    if (isFocused && hasVisualEffect(NeuTube::Display::Sphere::VE_DOT_CENTER)) {
      painter->drawPoint(QPointF(shiftedCenter.x(), shiftedCenter.y()));
    }

    if (isFocused && hasVisualEffect(NeuTube::Display::Sphere::VE_CROSS_CENTER))
    {
      painter->drawLine(QPointF(shiftedCenter.x() - 1, shiftedCenter.y()),
                        QPointF(shiftedCenter.x() + 1, shiftedCenter.y()));
      painter->drawLine(QPointF(shiftedCenter.x(), shiftedCenter.y() - 1),
                        QPointF(shiftedCenter.x(), shiftedCenter.y() + 1));
    }
  }

  bool drawingBoundBox = false;

  adjustedRadius = getAdjustedRadius(m_r);
  if (isSelected()) {
    drawingBoundBox = true;
    QColor color;
    color.setRgb(255, 255, 0);
    color.setAlphaF(alpha);
    pen.setColor(color);
    pen.setCosmetic(true);
    if (!visible && slice >= 0 && m_prevDisplaySlice >= 0) {
      double prevdc = fabs(painter->getZ(m_prevDisplaySlice) - shiftedCenter.z());
      double dc  = fabs(painter->getZ(slice) - shiftedCenter.z());
      if (prevdc > dc) {
        pen.setWidthF(pen.widthF() + 1.0);
      }
    }
  } else if (hasVisualEffect(NeuTube::Display::Sphere::VE_BOUND_BOX)) {
    drawingBoundBox = true;
    pen = oldPen;
    QColor color = oldPen.color();
    color.setAlphaF(1.0);
    pen.setColor(color);
    pen.setStyle(Qt::SolidLine);
    pen.setCosmetic(m_usingCosmeticPen);
  }


  if (drawingBoundBox) {
    QRectF rect;
    double halfSize = adjustedRadius;
    if (m_usingCosmeticPen) {
      halfSize += 0.5;
    }
    rect.setLeft(shiftedCenter.x() - halfSize);
    rect.setTop(shiftedCenter.y() - halfSize);
    rect.setWidth(halfSize * 2);
    rect.setHeight(halfSize * 2);

    painter->setBrush(Qt::NoBrush);
    pen.setWidthF(pen.widthF() * 0.5);
    if (visible) {
      pen.setStyle(Qt::SolidLine);
    } else {
      pen.setStyle(Qt::DotLine);
    }
    painter->setPen(pen);

#if 0 //for future versions
    QPen pen = oldPen;
    QVector<qreal> pattern;
    pattern << 1 << 2;
    pen.setDashPattern(pattern);
    painter->setPen(pen);
    painter->drawRect(rect);

    pen.setColor(Qt::black);
    pen.setDashOffset(1.5);
    painter->setPen(pen);
#endif

    //QPainter::CompositionMode oldMode = painter->compositionMode();
    //painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
    painter->drawRect(rect);
  }
#endif
}

void ZStackBall::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

bool ZStackBall::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);

  return false;
}

void ZStackBall::translate(double dx, double dy, double dz)
{
  m_center.translate(dx, dy, dz);
}

void ZStackBall::translate(const ZPoint &offset)
{
  translate(offset.x(), offset.y(), offset.z());
}

void ZStackBall::scaleCenter(double sx, double sy, double sz)
{
  m_center *= ZPoint(sx, sy, sz);
}

void ZStackBall::scale(double sx, double sy, double sz)
{
  scaleCenter(sx, sy, sz);
  m_r *= sqrt(sx * sy);
}

bool ZStackBall::hit(double x, double y, double z)
{
  return m_center.distanceTo(x, y, z) <= m_r;
}

bool ZStackBall::hit(double x, double y, NeuTube::EAxis axis)
{
  ZPoint shiftedCenter = m_center;
  shiftedCenter.shiftSliceAxis(axis);

  double dx = x - shiftedCenter.x();
  double dy = y - shiftedCenter.y();

  double d2 = dx * dx + dy * dy;

  return d2 <= m_r * m_r;
}


ZSTACKOBJECT_DEFINE_CLASS_NAME(ZStackBall)
