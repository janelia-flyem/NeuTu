#include "zstackobject.h"

#if _QT_APPLICATION_2
#include <QThread>
#include <QCoreApplication>
#endif

#include "geometry/zintcuboid.h"
#include "common/utilities.h"
#include "geometry/zcuboid.h"

//const char* ZStackObject::m_nodeAdapterId = "!NodeAdapter";
double ZStackObject::m_defaultPenWidth = 0.5;

ZStackObject::ZStackObject() : m_hitProtocal(EHitProtocol::HIT_DATA_POS),
  m_style(EDisplayStyle::SOLID), m_target(ETarget::WIDGET),
  m_zScale(1.0),
  m_zOrder(1), m_role(ZStackObjectRole::ROLE_NONE),
  m_visualEffect(neutu::display::VE_NONE)
{
  m_type = EType::UNIDENTIFIED;
  setSliceAxis(neutu::EAxis::Z);
  m_basePenWidth = m_defaultPenWidth;
  m_timeStamp = 0;
}

ZStackObject::~ZStackObject()
{
#if _QT_APPLICATION_2
  if (QCoreApplication::instance()) {
    if (QThread::currentThread() != QCoreApplication::instance()->thread()) {
      if (getType() != ZStackObject::EType::DVID_SYNAPSE &&
          getType() != ZStackObject::EType::OBJECT3D_SCAN) {
        std::cout << "Deteleted in separate threads!!!" << std::endl;
        std::cout << "Deconstructing " << this << ": " << getTypeName() << ", "
                  << getSource() << std::endl;
      }
    }
  }
#endif

#ifdef _DEBUG_2
  std::cout << "Deconstructing " << this << ": " << getTypeName() << ", "
            << getSource() << std::endl;
#endif
}

#define RETURN_TYPE_NAME(v, t) \
  if (v == EType::t) { \
    return NT_STR(t); \
  }

std::string ZStackObject::GetTypeName(EType type)
{
  RETURN_TYPE_NAME(type, UNIDENTIFIED);
  RETURN_TYPE_NAME(type, SWC);
  RETURN_TYPE_NAME(type, PUNCTUM);
  RETURN_TYPE_NAME(type, MESH);
  RETURN_TYPE_NAME(type, OBJ3D);
  RETURN_TYPE_NAME(type, STROKE);
  RETURN_TYPE_NAME(type, LOCSEG_CHAIN);
  RETURN_TYPE_NAME(type, CONN);
  RETURN_TYPE_NAME(type, OBJECT3D_SCAN);
  RETURN_TYPE_NAME(type, SPARSE_OBJECT);
  RETURN_TYPE_NAME(type, CIRCLE);
  RETURN_TYPE_NAME(type, STACK_BALL);
  RETURN_TYPE_NAME(type, STACK_PATCH);
  RETURN_TYPE_NAME(type, RECT2D);
  RETURN_TYPE_NAME(type, DVID_TILE);
  RETURN_TYPE_NAME(type, DVID_GRAY_SLICE);
  RETURN_TYPE_NAME(type, DVID_GRAY_SLICE_ENSEMBLE);
  RETURN_TYPE_NAME(type, DVID_TILE_ENSEMBLE);
  RETURN_TYPE_NAME(type, DVID_LABEL_SLICE);
  RETURN_TYPE_NAME(type, DVID_SPARSE_STACK);
  RETURN_TYPE_NAME(type, DVID_SPARSEVOL_SLICE);
  RETURN_TYPE_NAME(type, STACK);
  RETURN_TYPE_NAME(type, SWC_NODE);
  RETURN_TYPE_NAME(type, GRAPH_3D);
  RETURN_TYPE_NAME(type, PUNCTA);
  RETURN_TYPE_NAME(type, FLYEM_BOOKMARK);
  RETURN_TYPE_NAME(type, INT_CUBOID);
  RETURN_TYPE_NAME(type, LINE_SEGMENT);
  RETURN_TYPE_NAME(type, SLICED_PUNCTA);
  RETURN_TYPE_NAME(type, DVID_SYNAPSE);
  RETURN_TYPE_NAME(type, DVID_SYNAPE_ENSEMBLE);
  RETURN_TYPE_NAME(type, CUBE);
  RETURN_TYPE_NAME(type, DVID_ANNOTATION);
  RETURN_TYPE_NAME(type, FLYEM_TODO_ITEM);
  RETURN_TYPE_NAME(type, FLYEM_TODO_LIST);
  RETURN_TYPE_NAME(type, CROSS_HAIR);

  return std::to_string(neutu::EnumValue(type));
}

std::string ZStackObject::getTypeName() const
{
  return GetTypeName(getType());
}

bool ZStackObject::display(QPainter * /*painter*/, int /*z*/,
                           EDisplayStyle /*option*/, EDisplaySliceMode /*sliceMode*/,
                           neutu::EAxis /*sliceAxis*/) const
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

  if(m_selected) {
    for(auto callback: m_callbacks_on_selection)
    {
      callback(this);
    }
  } else {
    for(auto callback: m_callbacks_on_deselection)
    {
      callback(this);
    }
  }
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

bool ZStackObject::isSliceVisible(int /*z*/, neutu::EAxis /*axis*/) const
{
  return isVisible();
}

bool ZStackObject::hit(double /*x*/, double /*y*/, neutu::EAxis /*axis*/)
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
                       neutu::EAxis axis)
{
  switch (m_hitProtocal) {
  case EHitProtocol::HIT_DATA_POS:
    return hit(dataPos);
  case EHitProtocol::HIT_WIDGET_POS:
    return hitWidgetPos(widgetPos, axis);
  default:
    break;
  }

  return false;
}

bool ZStackObject::hitWidgetPos(
    const ZIntPoint &/*widgetPos*/, neutu::EAxis /*axis*/)
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

bool ZStackObject::IsSameSource(const std::string &s1, const std::string &s2)
{
  return (!s1.empty() && !s2.empty() && s1 == s2);
}

bool ZStackObject::IsSameClass(const std::string &s1, const std::string &s2)
{
  return IsSameSource(s1, s2);
}

/*
bool ZStackObject::isEmptyTree(const ZStackObject *obj)
{
  bool passed = false;
  if (obj != NULL) {
    if (obj->getType() == EType::SWC) {
      const ZSwcTree *tree = dynamic_cast<const ZSwcTree*>(obj);
      if (tree != NULL) {
        passed = tree->isEmpty();
      }
    }
  }

  return passed;
}
*/

bool ZStackObject::IsSelected(const ZStackObject *obj)
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

ZCuboid ZStackObject::getBoundBox() const
{
  return ZCuboid();
}

void ZStackObject::addVisualEffect(neutu::display::TVisualEffect ve)
{
  m_visualEffect |= ve;
}

void ZStackObject::removeVisualEffect(neutu::display::TVisualEffect ve)
{
  m_visualEffect &= ~ve;
}

void ZStackObject::setVisualEffect(neutu::display::TVisualEffect ve)
{
  m_visualEffect = ve;
}

bool ZStackObject::hasVisualEffect(neutu::display::TVisualEffect ve) const
{
  return m_visualEffect & ve;
}
