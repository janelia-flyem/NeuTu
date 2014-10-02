#if defined(_QT_GUI_USED_)
#include <QPainter>
#include <QDebug>
#endif

#include <math.h>
#include "zcircle.h"
#include "tz_math.h"
#include "zintpoint.h"


const ZCircle::TVisualEffect ZCircle::VE_NONE = 0;
const ZCircle::TVisualEffect ZCircle::VE_DASH_PATTERN = 1;
const ZCircle::TVisualEffect ZCircle::VE_BOUND_BOX = 2;
const ZCircle::TVisualEffect ZCircle::VE_NO_CIRCLE = 4;
const ZCircle::TVisualEffect ZCircle::VE_NO_FILL = 8;
const ZCircle::TVisualEffect ZCircle::VE_GRADIENT_FILL = 16;
const ZCircle::TVisualEffect ZCircle::VE_OUT_FOCUS_DIM = 32;

ZCircle::ZCircle() : m_visualEffect(ZCircle::VE_NONE)
{
  set(0, 0, 0, 1);
}

ZCircle::ZCircle(double x, double y, double z, double r) :
  m_visualEffect(ZCircle::VE_NONE)
{
  set(x, y, z, r);
}

void ZCircle::set(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_r = r;
}

void ZCircle::set(const ZIntPoint &center, double r)
{
  set(center.getX(), center.getY(), center.getZ(), r);
}

void ZCircle::set(const ZPoint &center, double r)
{
  set(center.x(), center.y(), center.z(), r);
}

//void ZCircle::display(QImage *image, int n, Display_Style style) const
//{
//#if defined(_QT_GUI_USED_)
//  QPainter painter(image);
//  painter.setPen(m_color);
//  display(&painter, n, style);
//#endif
//}

void ZCircle::display(ZPainter &painter, int n,
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

bool ZCircle::isCuttingPlane(double z, double r, double n)
{
  double h = fabs(z - n);
  if (r > h) {
    return true;
  } else if (iround(z) == iround(n)) {
    return true;
  }

  return false;
}

bool ZCircle::isCuttingPlane(double n)
{
  return isCuttingPlane(m_center.z(), m_r, n);
}

double ZCircle::getAdjustedRadius(double r) const
{
  double adjustedRadius = r;
  if (!m_usingCosmeticPen) {
    adjustedRadius += getPenWidth() * 0.5;
  }

  return adjustedRadius;
}

void ZCircle::displayHelper(ZPainter *painter, int stackFocus, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  double adjustedRadius = getAdjustedRadius(m_r);

  double dataFocus = stackFocus - painter->getOffset().z();
  bool visible = false;

  double alpha = 1.0;
  if (stackFocus == -1) {
    visible = true;
  } else {
    if (isCuttingPlane(m_center.z(), m_r, dataFocus)) {
      double h = fabs(m_center.z() - dataFocus);
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
        alpha = r / m_r;
        alpha *= alpha;
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

    const QBrush &oldBrush = painter->brush();
    const QPen &oldPen = painter->pen();
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
    painter->setBrush(oldBrush);
    painter->setPen(oldPen);
  }
#endif
}

void ZCircle::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

bool ZCircle::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);

  return false;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZCircle)
