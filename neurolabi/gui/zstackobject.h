#ifndef ZSTACKOBJECT_H
#define ZSTACKOBJECT_H

#include <functional>
#include <unordered_map>
#include <atomic>

#include "common/neutudefs.h"
#include "common/utilities.h"
#include "geometry/zintpoint.h"
#include "geometry/zaffinerect.h"
#include "data3d/defs.h"
#include "data3d/zstackobjecthandle.h"
#include "zqtheader.h"
#include "zstackobjectrole.h"

class ZPainter;
class ZIntCuboid;
class ZCuboid;

namespace neutu {
namespace data3d {
class DisplayConfig;
enum class EDisplayStyle;
enum class EDisplaySliceMode;
class ViewSpaceAlignedDisplayConfig;
}
}

/*!
 * \brief The abstract class of representing an 3D object
 *
 * ZStackObject is a class of objects in a stack. We call those objects stack
 * objects. The common properties of a stack object are:
 *
 *  Handle: each object has a unique handle that can be used as a reference of
 *          the object. Once an object is created, its handle cannot be changed.
 *  Source: a string telling where the object is from. It can be empty, meaning
 *          that the object is from unknown source.
 *  Visibility: the object is visible or not.
 *  Color: color of the object.
 *  Role: role of the object when it exists in a document.
 *  Selection: the object is selected or not.
 *  Type: type of an object. The types are predefined.
 *  Z Order: Z Order of the object to specify the order of painting for
 *           multiple objects on the same plane.
 *  Z Scale: Z Scale of the object.
 *  Target: painting target (canvas) of the object.
 *
 * Each object has a source string, which helps identifying its identity. The
 * source can be the file path where the object is loaded, it can also be a
 * string serving as an ID. If the source is empty, it means the object does
 * not have any source identification. How to use the source is up to the user,
 * but it is recommended to use ZStackObjectSourceFactory to generate source
 * for special purposes. There are some special characters put in the front
 * to specify the role of source IDs:
 *   '#': unique ID
 *   '#.': source class (the class part should always ends with '#')
 *   '!': reserved ID
 *
 * The class also provides the interface for object display and hit test.
 */
class ZStackObject
{
public:
  ZStackObject();
  ZStackObject(const ZStackObject &obj);
  ZStackObject(ZStackObject &&obj);
  virtual ~ZStackObject();

  using EType = neutu::data3d::EType;

  static std::string GetTypeName(EType type);

  enum class Palette_Color {
    BLUE = 0, GREEN, RED, ALPHA
  };

  using EDisplayStyle = neutu::data3d::EDisplayStyle;
  using EDisplaySliceMode = neutu::data3d::EDisplaySliceMode;
  using DisplayConfig = neutu::data3d::DisplayConfig;
  using ViewSpaceAlignedDisplayConfig = neutu::data3d::ViewSpaceAlignedDisplayConfig;

  enum class EHitProtocol {
    HIT_NONE, HIT_WIDGET_POS, HIT_DATA_POS
  };

  /*!
   * \brief Name of the type
   *
   * This function is mainly used for debugging.
   */
  std::string getTypeName() const;

  const ZStackObjectHandle& getHandle() const;

  virtual ZStackObject* clone() const;
  template<typename T>
  static T* Clone(T *obj);

  ZStackObject& operator=(const ZStackObject &obj);
  ZStackObject& operator=(ZStackObject &&obj);

//  virtual const std::string& className() const = 0;

  enum class ESelection {
    SELECT_SINGLE,
    SELECT_MULTIPLE,
    SELECT_TOGGLE,
    SELECT_TOGGLE_SINGLE, //toggle selection and deselect other objects
    DESELECT,
  };

  virtual void processHit(ESelection s);

  /*!
   * \brief Set the selection state
   */
  void setSelected(bool selected);

  /*!
   * \brief Get the selection state
   *
   * \return true iff the object is selected.
   */
  bool isSelected() const { return m_selected; }

  virtual void deselectSub() {};
  void deselect(bool recursive);

  using CallBack = std::function<void(ZStackObject*)>;
//  typedef void(*CallBack)(ZStackObject*);

  void addCallBackOnSelection(CallBack callback){
    m_selectionCallbacks.push_back(callback);}

  void addCallBackOnDeselection(CallBack callback){
    m_deselectionCallbacks.push_back(callback);}

  /*!
   * \brief The main painting function
   *
   * \return false if the object is not actually painted. Note the return value
   * is only a hint. Even if it returns true, it does not mean that the object
   * is actually painted.
   */
  virtual bool display(
      QPainter *painter, const DisplayConfig &config) const;

  bool isVisible(const DisplayConfig &config) const {
    return isVisible() && isVisible_inner(config);
  }

//  virtual void viewSpaceAlignedDisplay(
//      QPainter *painter, const ViewSpaceAlignedDisplayConfig &config) const;

  /*!
   * \brief Get an aligned object if possible.
   *
   * The caller is responsible to manage the memory of the returned pointer.
   */
  virtual ZStackObject* aligned(
      const ZAffinePlane &plane, neutu::EAxis sliceAxis) const;

  inline bool isVisible() const { return m_isVisible; }
  inline void setVisible(bool visible) { m_isVisible = visible; }
  inline void toggleVisible() { m_isVisible = !m_isVisible; }

//  inline void setDisplayStyle(zstackobject::EDisplayStyle style) { m_style = style; }
//  inline zstackobject::EDisplayStyle displayStyle() const { return m_style; }

  inline neutu::data3d::ETarget getTarget() const { return m_target; }
  inline void setTarget(neutu::data3d::ETarget target) { m_target = target; }

  virtual bool isSliceVisible(int z, neutu::EAxis axis) const;
  virtual bool isSliceVisible(
      int z, neutu::EAxis axis, const ZAffinePlane &plane) const;
  virtual bool isSliceVisible(const DisplayConfig &config) const;
  virtual bool isSliceVisible(
      const DisplayConfig &config, int canvasWidth, int canvasHeight) const;

  void setHittable(bool on);

  virtual bool hit(double x, double y, double z, int viewId);
  virtual bool hit(double x, double y, double z);
  virtual bool hit(const ZIntPoint &pt);
  virtual bool hit(const ZIntPoint &pt, int viewId);
  virtual bool hit(const ZIntPoint &dataPos, const ZIntPoint &widgetPos,
                   neutu::EAxis axis);
  virtual bool hit(double x, double y, neutu::EAxis axis);
  virtual bool hit(double x, double y, neutu::EAxis axis, int viewId);
  virtual bool hitWidgetPos(const ZIntPoint &widgetPos, neutu::EAxis axis);

  virtual inline const ZIntPoint& getHitPoint() const { return m_hitPoint; }

  void setHitFunc(std::function<bool(const ZStackObject*,double,double,double)> f);

  /*!
   * \brief Get bound box of the object.
   *
   * For compability purpose, it is set to take an output parameter instead of
   * returning the result.
   */
  virtual void boundBox(ZIntCuboid *box) const;
  virtual ZCuboid getBoundBox() const;
  virtual ZCuboid getBoundBox(int viewId) const;

  const QColor& getColor() const;
  void setColor(int red, int green, int blue);
  void setColor(int red, int green, int blue, int alpha);
  void setColor(const QColor &n);

  void setAlpha(int alpha);
  int getAlpha();

  double getAlphaF();
  double getRedF();
  double getGreenF();
  double getBlueF();

  bool isOpaque();

  inline static void SetDefaultPenWidth(double width) {
      m_defaultPenWidth = width;
  }

  inline static double GetDefaultPenWidth() {
    return m_defaultPenWidth;
  }

  double getPenWidth() const;

  double getBasePenWidth() const {
    return m_basePenWidth;
  }

  void setBasePenWidth(double width) {
    m_basePenWidth = width;
  }

  void useCosmeticPen(bool state) {
    m_usingCosmeticPen = state;
  }

  void setTimestamp(int64_t t) {
    m_timeStamp = t;
  }

  int64_t getTimestamp() const {
    return m_timeStamp;
  }

  void updateTimestamp();

  virtual void setLabel(uint64_t label);

  inline uint64_t getLabel() const {
    return m_uLabel;
  }

/*
  virtual void setILabel(int label);

  inline int getILabel() const {
    return m_label;
  }
*/
  inline const std::string &getSource() const { return m_source; }
  inline void setSource(const std::string &source) { m_source = source; }

  /*!
   * \brief Test if two objects are from the same source
   *
   * \return true if the two objects have the same type and non-empty source.
   */
  bool fromSameSource(const ZStackObject *obj) const;

  static bool fromSameSource(const ZStackObject *obj1, const ZStackObject *obj2);


  inline const std::string &getObjectId() const { return m_objectId; }
  inline void setObjectId(const std::string &id) { m_objectId = id; }

  bool hasSameId(const ZStackObject *obj) const;
  static bool hasSameId(const ZStackObject *obj1, const ZStackObject *obj2);

  inline const std::string &getObjectClass() const { return m_objectClass; }
  inline void setObjectClass(const std::string &id) { m_objectClass = id; }


  inline void setZScale(double scale) { m_zScale = scale; }

  /*!
   * \brief Z Order of the object
   *
   * The following values are prevserved for special purpose:
   *   0 - Main image
   *   > 0 Foreground objects (default value 1)
   *   < 0 - Background objects (default value -1)
   */
  inline int getZOrder() const { return m_zOrder; }
  void setZOrder(int order) { m_zOrder = order; }

  struct ZOrderLessThan {
    bool operator() (const ZStackObject &obj1, const ZStackObject &obj2) {
      return (obj1.getZOrder() < obj2.getZOrder());
    }

    template<typename ZStackObjectPtr>
    bool operator() (const ZStackObjectPtr &obj1, const ZStackObjectPtr &obj2) {
      return (obj1->getZOrder() < obj2->getZOrder());
    }
  };

  struct ZOrderBiggerThan {
    bool operator() (const ZStackObject &obj1, const ZStackObject &obj2) {
      if (neutu::EnumValue(obj1.getTarget()) ==
          neutu::EnumValue(obj2.getTarget())) {
        return (obj1.getZOrder() > obj2.getZOrder());
      }

      return  neutu::EnumValue(obj1.getTarget()) >
          neutu::EnumValue(obj2.getTarget());
    }

    bool operator() (const ZStackObject *obj1, const ZStackObject *obj2) {
      return ZOrderBiggerThan()(*obj1, *obj2);
//      return (obj1->getZOrder() > obj2->getZOrder());
    }
  };

  inline ZStackObject::EType getType() const {
    return m_type;
  }

  inline const ZStackObjectRole& getRole() const {
    return m_role;
  }

  inline bool hasRole() const {
    return !m_role.isNone();
  }

  inline bool hasRole(ZStackObjectRole::TRole role) const
  {
    return m_role.hasRole(role);
  }

  inline void setRole(ZStackObjectRole::TRole role) {
    m_role.setRole(role);
  }

  inline void addRole(ZStackObjectRole::TRole role) {
    m_role.addRole(role);
  }

  void removeRole(ZStackObjectRole::TRole role);

  /*
  static inline const char* getNodeAdapterId() {
    return m_nodeAdapterId;
  }
  */

  inline bool isSelectable() const {
    return m_isSelectable;
  }

  inline void setSelectable(bool state) {
    m_isSelectable = state;
  }

  inline bool isHittable() const {
    return m_hitProtocal != EHitProtocol::HIT_NONE && isVisible();
  }

  inline void setHitProtocal(EHitProtocol protocal) {
    m_hitProtocal = protocal;
  }

  void setHitPoint(const ZIntPoint &pt);

  inline bool isProjectionVisible() const {
    return m_projectionVisible;
  }

  inline void setProjectionVisible(bool visible) {
    m_projectionVisible = visible;
  }

  virtual void addVisualEffect(neutu::display::TVisualEffect ve);
  virtual void removeVisualEffect(neutu::display::TVisualEffect ve);
  virtual void setVisualEffect(neutu::display::TVisualEffect ve);
  bool hasVisualEffect(neutu::display::TVisualEffect ve) const;

  neutu::EAxis getSliceAxis() const { return m_sliceAxis; }
  void setSliceAxis(neutu::EAxis axis) { m_sliceAxis = axis; }

public:
//  static bool isEmptyTree(const ZStackObject *obj);
  static bool IsSameSource(const std::string &s1, const std::string &s2);
  static bool IsSameClass(const std::string &s1, const std::string &s2);
  static bool IsSelected(const ZStackObject *obj);
  template <typename T>
  static T* CastVoidPointer(void *p);

protected:
  struct DisplayTrace {
    int prevZ = 0;
    bool isValid = false;
  };
  void setPrevZ(int z) const;

  mutable std::function<bool(const ZStackObject*, double,double,double)>
  _hit = [](const ZStackObject*, double,double,double) {
    return false;
  };

  virtual bool isVisible_inner(const DisplayConfig &/*config*/) const {
    return true;
  }
  virtual bool display_inner(QPainter *painter, const DisplayConfig &config) const;

  mutable std::unordered_map<int,
  std::function<bool(const ZStackObject*, double,double,double)>> m_hitMap;

//  static uint64_t GetNextHandle();

protected:
  static double m_defaultPenWidth;

//protected:
  EHitProtocol m_hitProtocal = EHitProtocol::HIT_DATA_POS;
//  zstackobject::EDisplayStyle m_style = zstackobject::EDisplayStyle::SOLID;
  QColor m_color;
  neutu::data3d::ETarget m_target = neutu::data3d::ETarget::HD_OBJECT_CANVAS;
  EType m_type = EType::UNIDENTIFIED;
  neutu::data3d::ESpace m_coordSpace = neutu::data3d::ESpace::MODEL;
  double m_basePenWidth;

  double m_zScale = 1.0;
  std::string m_source;
  std::string m_objectClass;
  std::string m_objectId;
  uint64_t m_uLabel = 0;
//  uint64_t m_handle = 0;
  ZStackObjectHandle m_handle;
//  int m_label = -1;
  int m_zOrder = 1;
  int64_t m_timeStamp = 0;
  ZStackObjectRole m_role = ZStackObjectRole::ROLE_NONE;
  ZIntPoint m_hitPoint;
  neutu::EAxis m_sliceAxis;

  neutu::display::TVisualEffect m_visualEffect = neutu::display::VE_NONE;

  std::vector<CallBack> m_selectionCallbacks;
  std::vector<CallBack> m_deselectionCallbacks;

//  mutable int m_prevDisplaySlice = -1;
  mutable DisplayTrace m_displayTrace;

  bool m_selected = false;
  bool m_isSelectable = true;
  bool m_hittable = true;
  bool m_isVisible = true;
  bool m_projectionVisible = true;
  bool m_usingCosmeticPen = false;

//  static std::atomic<uint64_t> m_currentHandle;
//  static std::mutex m_handleMutex;
};

template <typename T>
T* ZStackObject::Clone(T *obj)
{
  if (obj) {
    return dynamic_cast<T*>(obj->clone());
  }

  return nullptr;
}

template <typename T>
T* ZStackObject::CastVoidPointer(void *p)
{
  return dynamic_cast<T*>(static_cast<ZStackObject*>(p));
}
#if 0
#define ZSTACKOBJECT_DEFINE_CLASS_NAME(c) \
  const std::string& c::className() const {\
    static const std::string name = #c;\
    return name; \
  }
#endif

#endif // ZSTACKOBJECT_H
