#ifndef ZSTACKOBJECT_H
#define ZSTACKOBJECT_H

#include "zqtheader.h"
#include "zpainter.h"
#include "zstackobjectrole.h"

/*!
 * \brief The abstract class of representing an 3D object
 *
 * ZStackObject is a class of objects in a stack. We call those objects stack
 * objects. The common properties of a stack object are:
 *
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
  virtual ~ZStackObject() {}

  enum EType {
    TYPE_UNIDENTIFIED = 0, //Unidentified type
    TYPE_SWC,
    TYPE_PUNCTUM,
    TYPE_OBJ3D,
    TYPE_STROKE,
    TYPE_LOCSEG_CHAIN,
    TYPE_CONN,
    TYPE_OBJECT3D_SCAN,
    TYPE_SPARSE_OBJECT,
    TYPE_CIRCLE,
    TYPE_STACK_BALL,
    TYPE_STACK_PATCH,
    TYPE_RECT2D,
    TYPE_DVID_TILE,
    TYPE_DVID_TILE_ENSEMBLE
  };

  enum Palette_Color {
    BLUE = 0, GREEN, RED, ALPHA
  };

  enum EDisplayStyle {
    NORMAL, SOLID, BOUNDARY, SKELETON
  };

  enum ETarget {
    STACK_CANVAS, OBJECT_CANVAS, WIDGET, TILE_CANVAS
  };

  enum EDisplaySliceMode {
    DISPLAY_SLICE_PROJECTION, //Display Z-projection of the object
    DISPLAY_SLICE_SINGLE      //Display a cross section of the object
  };

  /*!
   * \brief Name of the class
   *
   * This function is mainly used for debugging.
   */
  virtual const std::string& className() const = 0;

  /*!
   * \brief Set the selection state
   */
  void setSelected(bool selected) { m_selected = selected; }

  /*!
   * \brief Get the selection state
   *
   * \return true iff the object is selected.
   */
  bool isSelected() const { return m_selected; }


  /*!
   * \brief Display an object to widget
   *
   * \a painter is expected to be restored after painting
   */
  virtual void display(
      ZPainter &painter, int slice, EDisplayStyle option) const = 0;

  /* For special painting when ZPainter cannot be created */
  virtual void display(
      QPainter *painter, int z, EDisplayStyle option,
      EDisplaySliceMode sliceMode) const;

  inline bool isVisible() const { return m_isVisible; }
  inline void setVisible(bool visible) { m_isVisible = visible; }
  inline void setDisplayStyle(EDisplayStyle style) { m_style = style; }
  inline EDisplayStyle displayStyle() const { return m_style; }

  inline ETarget getTarget() const { return m_target; }
  inline void setTarget(ETarget target) { m_target = target; }

  virtual bool isSliceVisible(int z) const;

  virtual bool hit(double x, double y, double z);
  virtual bool hit(double x, double y);

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

  inline static void setDefaultPenWidth(double width) {
      m_defaultPenWidth = width;
  }

  inline static double getDefaultPenWidth() {
    return m_defaultPenWidth;
  }

  double getPenWidth() const;

  void useCosmeticPen(bool state) {
    m_usingCosmeticPen = state;
  }

  inline std::string getSource() const { return m_source; }
  inline void setSource(const std::string &source) { m_source = source; }

  /*!
   * \brief Test if two objects are from the same source
   *
   * \return true if the two objects have the same type and non-empty source.
   */
  bool fromSameSource(const ZStackObject *obj) const;

  static bool fromSameSource(const ZStackObject *obj1, const ZStackObject *obj2);

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

  struct ZOrderCompare {
    bool operator() (const ZStackObject &obj1, const ZStackObject &obj2) {
      return (obj1.getZOrder() < obj2.getZOrder());
    }

    bool operator() (const ZStackObject *obj1, const ZStackObject *obj2) {
      return (obj1->getZOrder() < obj2->getZOrder());
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

  static inline const char* getNodeAdapterId() {
    return m_nodeAdapterId;
  }

  inline bool isHittable() const {
    return m_isHittable;
  }

  inline void setHittable(bool state) {
    m_isHittable = state;
  }

public:
  static bool isEmptyTree(const ZStackObject *obj);
  static bool isSameSource(const std::string &s1, const std::string &s2);
  static bool isSelected(const ZStackObject *obj);
  template <typename T>
  static T* CastVoidPointer(void *p);

protected:
  bool m_selected;
  bool m_isVisible;
  bool m_isHittable;
  EDisplayStyle m_style;
  QColor m_color;
  ETarget m_target;
  static double m_defaultPenWidth;
  bool m_usingCosmeticPen;
  double m_zScale;
  std::string m_source;
  int m_zOrder;
  EType m_type;
  ZStackObjectRole m_role;

  static const char *m_nodeAdapterId;
};

template <typename T>
T* ZStackObject::CastVoidPointer(void *p)
{
  return dynamic_cast<T*>(static_cast<ZStackObject*>(p));
}

#define ZSTACKOBJECT_DEFINE_CLASS_NAME(c) \
  const std::string& c::className() const {\
    static const std::string name = #c;\
    return name; \
  }

#endif // ZSTACKOBJECT_H
