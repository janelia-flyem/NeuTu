#include "zstackobject.h"
#include "tz_cdefs.h"
#include "zswctree.h"
#include "zintcuboid.h"

//const char* ZStackObject::m_nodeAdapterId = "!NodeAdapter";

ZStackObject::ZStackObject() : m_selected(false), m_isSelectable(true),
  m_isVisible(true), m_isHittable(true), m_projectionVisible(true),
  m_style(SOLID), m_target(TARGET_WIDGET), m_usingCosmeticPen(false), m_zScale(1.0),
  m_zOrder(1), m_role(ZStackObjectRole::ROLE_NONE),
  m_visualEffect(NeuTube::Display::VE_NONE), m_prevDisplaySlice(-1)
{
  m_type = GetType();
  setSliceAxis(NeuTube::Z_AXIS);
}

ZStackObject::~ZStackObject()
{
#ifdef _DEBUG_2
  std::cout << "Deconstructing " << this << ": " << getType() << ", "
            << getSource() << std::endl;
#endif
}

double ZStackObject::m_defaultPenWidth = 0.5;

bool ZStackObject::display(QPainter */*painter*/, int /*z*/,
                           EDisplayStyle /*option*/, EDisplaySliceMode /*sliceMode*/,
                           NeuTube::EAxis /*sliceAxis*/) const
{
  return false;
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

bool ZStackObject::isSliceVisible(int /*z*/, NeuTube::EAxis /*axis*/) const
{
  return isVisible();
}

bool ZStackObject::hit(double /*x*/, double /*y*/, NeuTube::EAxis /*axis*/)
{
  return false;
}

bool ZStackObject::hit(double /*x*/, double /*y*/, double /*z*/)
{
  return false;
}

void ZStackObject::setHitPoint(const ZIntPoint &pt)
{
  m_hitPoint = pt;
}

bool ZStackObject::fromSameSource(const ZStackObject *obj) const
{
  bool sameSource = false;
  if (obj != NULL) {
    if ((getType() == obj->getType()) &&
        !getSource().empty() && !obj->getSource().empty()) {
      if (getSource() == obj->getSource()) {
        sameSource = true;
      }
    }
  }

  return sameSource;
}

bool ZStackObject::hasSameId(const ZStackObject *obj) const
{
  bool same = false;
  if (obj != NULL) {
    if (!getObjectId().empty() && !obj->getObjectId().empty()) {
      if (getObjectId() == obj->getObjectId()) {
        same = true;
      }
    }
  }

  return same;
}

bool ZStackObject::fromSameSource(
    const ZStackObject *obj1, const ZStackObject *obj2)
{
  bool sameSource = false;
  if (obj1 != NULL) {
    sameSource = obj1->fromSameSource(obj2);
  }

  return sameSource;
}

bool ZStackObject::hasSameId(const ZStackObject *obj1, const ZStackObject *obj2)
{
  bool same = false;
  if (obj1 != NULL) {
    same = obj1->hasSameId(obj2);
  }

  return same;
}

bool ZStackObject::isSameSource(const std::string &s1, const std::string &s2)
{
  return (!s1.empty() && !s2.empty() && s1 == s2);
}

bool ZStackObject::isSameClass(const std::string &s1, const std::string &s2)
{
  return (!s1.empty() && !s2.empty() && s1 == s2);
}

bool ZStackObject::isEmptyTree(const ZStackObject *obj)
{
  bool passed = false;
  if (obj != NULL) {
    if (obj->getType() == ZStackObject::TYPE_SWC) {
      const ZSwcTree *tree = dynamic_cast<const ZSwcTree*>(obj);
      if (tree != NULL) {
        passed = tree->isEmpty();
      }
    }
  }

  return passed;
}

bool ZStackObject::isSelected(const ZStackObject *obj)
{
  if (obj != NULL) {
    return obj->isSelected();
  }

  return false;
}

void ZStackObject::getBoundBox(ZIntCuboid *box) const
{
  if (box != NULL) {
    *box = ZIntCuboid();
  }
}

void ZStackObject::addVisualEffect(NeuTube::Display::TVisualEffect ve)
{
  m_visualEffect |= ve;
}

void ZStackObject::removeVisualEffect(NeuTube::Display::TVisualEffect ve)
{
  m_visualEffect &= ~ve;
}

void ZStackObject::setVisualEffect(NeuTube::Display::TVisualEffect ve)
{
  m_visualEffect = ve;
}

bool ZStackObject::hasVisualEffect(NeuTube::Display::TVisualEffect ve) const
{
  return m_visualEffect & ve;
}
