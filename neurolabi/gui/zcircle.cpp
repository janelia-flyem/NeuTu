#if defined(_QT_GUI_USED_)
#include <QPainter>
#endif

#include <math.h>
#include "zcircle.h"
#include "tz_math.h"

ZCircle::ZCircle() : m_visualEffect(NO_VISUAL_EFFECT)
{
  set(0, 0, 0, 1);
}

ZCircle::ZCircle(double x, double y, double z, double r) :
  m_visualEffect(NO_VISUAL_EFFECT)
{
  set(x, y, z, r);
}

void ZCircle::set(double x, double y, double z, double r)
{
  m_center.set(x, y, z);
  m_r = r;
}

//void ZCircle::display(QImage *image, int n, Display_Style style) const
//{
//#if defined(_QT_GUI_USED_)
//  QPainter painter(image);
//  painter.setPen(m_color);
//  display(&painter, n, style);
//#endif
//}

void ZCircle::display(QPainter &painter, int n,
                      ZStackDrawable::Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if _QT_GUI_USED_
  QPen pen(m_color, m_defaultPenWidth);
  switch (m_visualEffect) {
  case DASH_PATTERN:
    pen.setStyle(Qt::DotLine);
    break;
  default:
    break;
  }

  painter.setPen(pen);

  display(&painter, n, style);
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

void ZCircle::display(QPainter *painter, int n, Display_Style style) const
{
  UNUSED_PARAMETER(style);
#if defined(_QT_GUI_USED_)
  if (n == -1) {
    double adjustedRadius = m_r + m_defaultPenWidth * 0.5;
    painter->drawEllipse(QPointF(m_center.x(), m_center.y()),
                         adjustedRadius, adjustedRadius);
  } else {
    if (isCuttingPlane(m_center.z(), m_r, n)) {
      double h = fabs(m_center.z() - n);
      if (m_r > h) {
        double r = sqrt(m_r * m_r - h * h);
        double adjustedRadius = r + m_defaultPenWidth * 0.5;
        painter->drawEllipse(QPointF(m_center.x(), m_center.y()), adjustedRadius,
                             adjustedRadius);
      } else { //too small, show at least one plane
        double adjustedRadius = m_defaultPenWidth * 0.5;
        painter->drawEllipse(QPointF(m_center.x(), m_center.y()), adjustedRadius,
                             adjustedRadius);
      }
    }
  }
#endif
}

void ZCircle::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

void ZCircle::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

ZINTERFACE_DEFINE_CLASS_NAME(ZCircle)
