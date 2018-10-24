#include "zstackobject.h"
#include "tz_cdefs.h"
#include "zswctree.h"
#include "zintcuboid.h"
#include "core/utilities.h"

//const char* ZStackObject::m_nodeAdapterId = "!NodeAdapter";
double ZStackObject::m_defaultPenWidth = 0.5;

ZStackObject::ZStackObject() : m_selected(false), m_isSelectable(true),
  m_isVisible(true), m_hitProtocal(HIT_DATA_POS), m_projectionVisible(true),
  m_style(SOLID), m_target(TARGET_WIDGET), m_usingCosmeticPen(false), m_zScale(1.0),
  m_zOrder(1), m_role(ZStackObjectRole::ROLE_NONE),
  m_visualEffect(neutube::display::VE_NONE), m_prevDisplaySlice(-1)
{
  m_type = TYPE_UNIDENTIFIED;
  setSliceAxis(neutube::EAxis::Z);
  m_basePenWidth = m_defaultPenWidth;
  m_timeStamp = 0;
}

ZStackObject::~ZStackObject()
{
#ifdef _DEBUG_2
  std::cout << "Deconstructing " << this << ": " << getType() << ", "
            << getSource() << std::endl;
#endif
}

#define GET_TYPE_NAME(v, t) \
  if (v == t) { \
    return NT_STR(t); \
  }

std::string ZStackObject::GetTypeName(EType type)
{
  GET_TYPE_NAME(type, TYPE_UNIDENTIFIED);
  GET_TYPE_NAME(type, TYPE_SWC);
  GET_TYPE_NAME(type, TYPE_PUNCTUM);
  GET_TYPE_NAME(type, TYPE_MESH);
  GET_TYPE_NAME(type, TYPE_OBJ3D);
  GET_TYPE_NAME(type, TYPE_STROKE);
  GET_TYPE_NAME(type, TYPE_LOCSEG_CHAIN);
  GET_TYPE_NAME(type, TYPE_CONN);
  GET_TYPE_NAME(type, TYPE_OBJECT3D_SCAN);
  GET_TYPE_NAME(type, TYPE_SPARSE_OBJECT);
  GET_TYPE_NAME(type, TYPE_CIRCLE);
  GET_TYPE_NAME(type, TYPE_STACK_BALL);
  GET_TYPE_NAME(type, TYPE_STACK_PATCH);
  GET_TYPE_NAME(type, TYPE_RECT2D);
  GET_TYPE_NAME(type, TYPE_DVID_TILE);
  GET_TYPE_NAME(type, TYPE_DVID_GRAY_SLICE);
  GET_TYPE_NAME(type, TYPE_DVID_TILE_ENSEMBLE);
  GET_TYPE_NAME(type, TYPE_DVID_LABEL_SLICE);
  GET_TYPE_NAME(type, TYPE_DVID_SPARSE_STACK);
  GET_TYPE_NAME(type, TYPE_DVID_SPARSEVOL_SLICE);
  GET_TYPE_NAME(type, TYPE_STACK);
  GET_TYPE_NAME(type, TYPE_SWC_NODE);
  GET_TYPE_NAME(type, TYPE_3D_GRAPH);
  GET_TYPE_NAME(type, TYPE_PUNCTA);
  GET_TYPE_NAME(type, TYPE_FLYEM_BOOKMARK);
  GET_TYPE_NAME(type, TYPE_INT_CUBOID);
  GET_TYPE_NAME(type, TYPE_LINE_SEGMENT);
  GET_TYPE_NAME(type, TYPE_SLICED_PUNCTA);
  GET_TYPE_NAME(type, TYPE_DVID_SYNAPSE);
  GET_TYPE_NAME(type, TYPE_DVID_SYNAPE_ENSEMBLE);
  GET_TYPE_NAME(type, TYPE_3D_CUBE);
  GET_TYPE_NAME(type, TYPE_DVID_ANNOTATION);
  GET_TYPE_NAME(type, TYPE_FLYEM_TODO_ITEM);
  GET_TYPE_NAME(type, TYPE_FLYEM_TODO_LIST);
  GET_TYPE_NAME(type, TYPE_CROSS_HAIR);

  return std::to_string(type);
}

bool ZStackObject::display(QPainter * /*painter*/, int /*z*/,
                           EDisplayStyle /*option*/, EDisplaySliceMode /*sliceMode*/,
                           neutube::EAxis /*sliceAxis*/) const
{
  return false;
}

void ZStackObject::setLabel(uint64_t label)
{
  m_uLabel = label;
}

void ZStackObject::setSelected(bool selected)
{
  m_selected = selected;
}

void ZStackObject::setColor(int red, int green, int blue)
{
#if defined(_QT_GUI_USED_)
  setColor(QColor(red, green, blue, m_color.alpha()));
#else
  UNUSED_PARAMETER(red);
  UNUSED_PARAMETER(green);
  UNUSED_PARAMETER(blue);
#endif

//#if defined(_QT_GUI_USED_)
//  m_color.setRed(red);
//  m_color.setGreen(green);
//  m_color.setBlue(blue);
//#else
//  UNUSED_PARAMETER(red);
//  UNUSED_PARAMETER(green);
//  UNUSED_PARAMETER(blue);
//#endif
}

void ZStackObject::setColor(int red, int green, int blue, int alpha)
{
#if defined(_QT_GUI_USED_)
  setColor(QColor(red, green, blue, alpha));
#endif

//#if defined(_QT_GUI_USED_)
//  m_color.setRed(red);
//  m_color.setGreen(green);
//  m_color.setBlue(blue);
//  m_color.setAlpha(alpha);
//#else
//  UNUSED_PARAMETER(red);
//  UNUSED_PARAMETER(green);
//  UNUSED_PARAMETER(blue);
//  UNUSED_PARAMETER(alpha);
//#endif
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
  setColor(QColor(m_color.red(), m_color.green(), m_color.blue(), alpha));
//  m_color.setAlpha(alpha);
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
  double width = m_basePenWidth;

  if (m_usingCosmeticPen) {
    width += 1.0;
  }

  return width;
}

bool ZStackObject::isSliceVisible(int /*z*/, neutube::EAxis /*axis*/) const
{
  return isVisible();
}

bool ZStackObject::hit(double /*x*/, double /*y*/, neutube::EAxis /*axis*/)
{
  return false;
}

bool ZStackObject::hit(double /*x*/, double /*y*/, double /*z*/)
{
  return false;
}

bool ZStackObject::hit(const ZIntPoint &pt)
{
  return hit(pt.getX(), pt.getY(), pt.getZ());
}

bool ZStackObject::hit(const ZIntPoint &dataPos, const ZIntPoint &widgetPos,
                       neutube::EAxis axis)
{
  switch (m_hitProtocal) {
  case HIT_DATA_POS:
    return hit(dataPos);
  case HIT_WIDGET_POS:
    return hitWidgetPos(widgetPos, axis);
  default:
    break;
  }

  return false;
}

bool ZStackObject::hitWidgetPos(
    const ZIntPoint &/*widgetPos*/, neutube::EAxis /*axis*/)
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

void ZStackObject::removeRole(ZStackObjectRole::TRole role)
{
  m_role.removeRole(role);
}

void ZStackObject::boundBox(ZIntCuboid *box) const
{
  if (box != NULL) {
    *box = ZIntCuboid();
  }
}

void ZStackObject::addVisualEffect(neutube::display::TVisualEffect ve)
{
  m_visualEffect |= ve;
}

void ZStackObject::removeVisualEffect(neutube::display::TVisualEffect ve)
{
  m_visualEffect &= ~ve;
}

void ZStackObject::setVisualEffect(neutube::display::TVisualEffect ve)
{
  m_visualEffect = ve;
}

bool ZStackObject::hasVisualEffect(neutube::display::TVisualEffect ve) const
{
  return m_visualEffect & ve;
}
