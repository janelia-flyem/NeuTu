#include "zstackobject.h"
#include "tz_cdefs.h"

const char* ZStackObject::m_nodeAdapterId = "!NodeAdapter";

ZStackObject::ZStackObject() : m_selected(false), m_isVisible(true),
  m_style(SOLID), m_target(WIDGET), m_usingCosmeticPen(false), m_zScale(1.0),
  m_zOrder(0), m_type(TYPE_UNIDENTIFIED)
{
}

double ZStackObject::m_defaultPenWidth = 0.5;

void ZStackObject::display(QPainter */*painter*/, int /*z*/,
                           Display_Style /*option*/) const
{
}

void ZStackObject::setColor(int red, int green, int blue) {
#if defined(_QT_GUI_USED_)
  m_color.setRed(red);
  m_color.setGreen(green);
  m_color.setBlue(blue);
#else
  UNUSED_PARAMETER(red);
  UNUSED_PARAMETER(green);
  UNUSED_PARAMETER(blue);
#endif
}

void ZStackObject::setColor(int red, int green, int blue, int alpha) {
#if defined(_QT_GUI_USED_)
  m_color.setRed(red);
  m_color.setGreen(green);
  m_color.setBlue(blue);
  m_color.setAlpha(alpha);
#else
  UNUSED_PARAMETER(red);
  UNUSED_PARAMETER(green);
  UNUSED_PARAMETER(blue);
  UNUSED_PARAMETER(alpha);
#endif
}

void ZStackObject::setColor(const QColor &n) {
#if defined(_QT_GUI_USED_)
  m_color = n;
#else
  UNUSED_PARAMETER(&n);
#endif
}

void ZStackObject::setAlpha(int alpha) {
#if defined(_QT_GUI_USED_)
  m_color.setAlpha(alpha);
#else
  UNUSED_PARAMETER(alpha);
#endif
}

int ZStackObject::getAlpha() {
#if defined(_QT_GUI_USED_)
  return m_color.alpha();
#else
  return 0;
#endif
}

double ZStackObject::getAlphaF() {
#if defined(_QT_GUI_USED_)
  return m_color.alphaF();
#else
  return 0.0;
#endif
}

double ZStackObject::getRedF() {
#if defined(_QT_GUI_USED_)
  return m_color.redF();
#else
  return 0.0;
#endif
}

double ZStackObject::getGreenF() {
#if defined(_QT_GUI_USED_)
  return m_color.greenF();
#else
  return 0.0;
#endif
}

double ZStackObject::getBlueF() {
#if defined(_QT_GUI_USED_)
  return m_color.blueF();
#else
  return 0.0;
#endif
}

bool ZStackObject::isOpaque() {
  return getAlpha() == 255;
}

const QColor& ZStackObject::getColor() const
{
  return m_color;
}

double ZStackObject::getPenWidth() const
{
  double width = getDefaultPenWidth();

  if (m_usingCosmeticPen) {
    width += 2.0;
  }

  return width;
}

bool ZStackObject::isSliceVisible(int /*z*/) const
{
  return isVisible();
}
