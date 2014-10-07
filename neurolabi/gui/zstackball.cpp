#include "zstackball.h"

#include <math.h>
#include "tz_math.h"
#include "zintpoint.h"


const ZStackBall::TVisualEffect ZStackBall::VE_NONE = 0;
const ZStackBall::TVisualEffect ZStackBall::VE_DASH_PATTERN = 1;
const ZStackBall::TVisualEffect ZStackBall::VE_BOUND_BOX = 2;
const ZStackBall::TVisualEffect ZStackBall::VE_NO_CIRCLE = 4;
const ZStackBall::TVisualEffect ZStackBall::VE_NO_FILL = 8;
const ZStackBall::TVisualEffect ZStackBall::VE_GRADIENT_FILL = 16;
const ZStackBall::TVisualEffect ZStackBall::VE_OUT_FOCUS_DIM = 32;

ZStackBall::ZStackBall() : m_visualEffect(ZStackBall::VE_NONE)
{
  set(0, 0, 0, 1);
}

ZStackBall::ZStackBall(double x, double y, double z, double r) :
  m_visualEffect(ZStackBall::VE_NONE)
{
  set(x, y, z, r);
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

void ZStackBall::display(ZPainter &painter, int n,
                      ZStackObject::Display_Style style) const
{
  if (!isVisible()) {
    return;
  }

  UNUSED_PARAMETER(style);
#if _QT_GUI_USED_
  QPen pen(m_color, getPenWidth());
  pen.setCosmetic(m_usingCosmeticPen);

  if (hasVisualEffect(VE_DASH_PATTERN)) {
    pen.setStyle(Qt::DotLine);
  }

  painter.setPen(pen);

  //qDebug() << "Internal color: " << m_color;
  const QBrush &oldBrush = painter.brush();
  if (hasVisualEffect(VE_GRADIENT_FILL)) {
    QRadialGradient gradient(50, 50, 50, 50, 50);
    gradient.setColorAt(0, QColor::fromRgbF(0, 1, 0, 1));
    gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));

    /*
    QBrush brush(gradient);
    brush.setColor(m_color);
    brush.setStyle(Qt::RadialGradientPattern);
    painter.setBrush(brush);
    */
    //painter.setBrush(m_color);
    //painter.setBrush(QBrush(m_color, Qt::RadialGradientPattern));
  } else {
    if (hasVisualEffect(VE_NO_FILL)) {
      painter.setBrush(Qt::NoBrush);
    }
  }
  displayHelper(&painter, n, style);
  painter.setBrush(oldBrush);
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

bool ZStackBall::isCuttingPlane(double n, double zScale) const
{
  return isCuttingPlane(m_center.z(), m_r, n, zScale);
}

bool ZStackBall::isSliceVisible(int z) const
{
  if (isVisible()) {
    if (z == -1 || isCuttingPlane(z, m_zScale)) {
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

void ZStackBall::displayHelper(ZPainter *painter, int stackFocus, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  double adjustedRadius = getAdjustedRadius(m_r);

  double dataFocus = stackFocus - painter->getOffset().z();
  bool visible = false;

  const QBrush &oldBrush = painter->brush();
  const QPen &oldPen = painter->pen();
  double alpha = oldPen.color().alphaF();

  if (stackFocus == -1) {
    visible = true;
  } else {
    if (isCuttingPlane(m_center.z(), m_r, dataFocus, m_zScale)) {
      double h = fabs(m_center.z() - dataFocus) / m_zScale;
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
      if (hasVisualEffect(VE_OUT_FOCUS_DIM)) {
        alpha *= r * r / m_r / m_r;
        //alpha *= alpha;
      }
    }
  }

  if (visible) {
    if (!hasVisualEffect(VE_NO_CIRCLE)) {
      //qDebug() << painter->brush().color();
      QColor color = painter->pen().color();
      color.setAlphaF(alpha);
      painter->setPen(color);
      painter->drawEllipse(QPointF(m_center.x(), m_center.y()),
                           adjustedRadius, adjustedRadius);
    }
  }

  if (hasVisualEffect(VE_BOUND_BOX)) {
    QRectF rect;
    rect.setLeft(m_center.x() - adjustedRadius);
    rect.setTop(m_center.y() - adjustedRadius);
    rect.setWidth(adjustedRadius + adjustedRadius);
    rect.setHeight(adjustedRadius + adjustedRadius);

    painter->setBrush(Qt::NoBrush);

    QPen pen = oldPen;
    pen.setStyle(Qt::SolidLine);
    pen.setCosmetic(m_usingCosmeticPen);
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

    //painter->setCompositionMode(oldMode);
  }

  painter->setBrush(oldBrush);
  painter->setPen(oldPen);
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

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZStackBall)
