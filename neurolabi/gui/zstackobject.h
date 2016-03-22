#ifndef ZSTACKOBJECT_H
#define ZSTACKOBJECT_H

#include "neutube_def.h"
#include "zqtheader.h"
//#include "zpainter.h"
#include "zstackobjectrole.h"
#include "zintpoint.h"
#include "zsharedpointer.h"

class ZPainter;
class ZIntCuboid;


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
  virtual ~ZStackObject();

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
    TYPE_DVID_GRAY_SLICE,
    TYPE_DVID_TILE_ENSEMBLE,
    TYPE_DVID_LABEL_SLICE,
    TYPE_DVID_SPARSE_STACK,
    TYPE_DVID_SPARSEVOL_SLICE,
    TYPE_STACK,
    TYPE_SWC_NODE,
    TYPE_3D_GRAPH,
    TYPE_PUNCTA,
    TYPE_FLYEM_BOOKMARK,
    TYPE_INT_CUBOID,
    TYPE_LINE_SEGMENT,
    TYPE_SLICED_PUNCTA,
    TYPE_DVID_SYNAPSE,
    TYPE_DVID_SYNAPE_ENSEMBLE,
    TYPE_DVID_ANNOTATION,
    TYPE_FLYEM_TODO_ITEM,
    TYPE_FLYEM_TODO_LIST
  };

  enum Palette_Color {
    BLUE = 0, GREEN, RED, ALPHA
  };

  enum EDisplayStyle {
    NORMAL, SOLID, BOUNDARY, SKELETON
  };

  enum ETarget {
    TARGET_STACK_CANVAS, TARGET_OBJECT_CANVAS, TARGET_WIDGET, TARGET_TILE_CANVAS,
    TARGET_3D_ONLY
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
   * \a painter stores the painting status changed by the function. The
   * painting parameters, including pen and brush, of \a painter is expected to
   * be restored after painting.
   *
   * \param slice Index of the slice to display. The index is the offset from
   *    the current Z position to the start Z position in the painter. If it is
   *    negative, it means that the projection mode has been turned on at the
   *    current slice -(\a slice + 1).
   */
  virtual void display(
      ZPainter &painter, int slice, EDisplayStyle option,
      NeuTube::EAxis sliceAxis) const = 0;

  /*!
   * For special painting when ZPainter cannot be created
   *
   * \return false if nothing is painted. But returning true does not mean
   *         if there is something painted.
   */
  virtual bool display(
      QPainter *painter, int z, EDisplayStyle option,
      EDisplaySliceMode sliceMode, NeuTube::EAxis sliceAxis) const;

  inline bool isVisible() const { return m_isVisible; }
  inline void setVisible(bool visible) { m_isVisible = visible; }
  inline void toggleVisible() { m_isVisible = !m_isVisible; }

  inline void setDisplayStyle(EDisplayStyle style) { m_style = style; }
  inline EDisplayStyle displayStyle() const { return m_style; }

  inline ETarget getTarget() const { return m_target; }
  inline void setTarget(ETarget target) { m_target = target; }

  virtual bool isSliceVisible(int z, NeuTube::EAxis axis) const;

  virtual bool hit(double x, double y, double z);
  virtual bool hit(double x, double y, NeuTube::EAxis axis);
  virtual inline const ZIntPoint& getHitPoint() const { return m_hitPoint; }

  /*!
   * \brief Get bound box of the object.
   *
   * For compability purpose, it is set to take an output parameter instead of
   * returning the result.
   */
  virtual void getBoundBox(ZIntCuboid *box) const;

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


  inline std::string getObjectId() const { return m_objectId; }
  inline void setObjectId(const std::string &id) { m_objectId = id; }

  bool hasSameId(const ZStackObject *obj) const;
  static bool hasSameId(const ZStackObject *obj1, const ZStackObject *obj2);

  inline std::string getObjectClass() const { return m_objectClass; }
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

    bool operator() (const ZStackObject *obj1, const ZStackObject *obj2) {
      return (obj1->getZOrder() < obj2->getZOrder());
    }
  };

  struct ZOrderBiggerThan {
    bool operator() (const ZStackObject &obj1, const ZStackObject &obj2) {
      return (obj1.getZOrder() > obj2.getZOrder());
    }

    bool operator() (const ZStackObject *obj1, const ZStackObject *obj2) {
      return (obj1->getZOrder() > obj2->getZOrder());
    }
  };

  inline ZStackObject::EType getType() const {
    return m_type;
  }

  static ZStackObject::EType GetType() {
    return ZStackObject::TYPE_UNIDENTIFIED;
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
    return m_isHittable && isVisible();
  }

  inline void setHittable(bool state) {
    m_isHittable = state;
  }

  void setHitPoint(const ZIntPoint &pt);

  inline bool isProjectionVisible() const {
    return m_projectionVisible;
  }

  inline void setProjectionVisible(bool visible) {
    m_projectionVisible = visible;
  }

  virtual void addVisualEffect(NeuTube::Display::TVisualEffect ve);
  virtual void removeVisualEffect(NeuTube::Display::TVisualEffect ve);
  virtual void setVisualEffect(NeuTube::Display::TVisualEffect ve);
  bool hasVisualEffect(NeuTube::Display::TVisualEffect ve) const;

  NeuTube::EAxis getSliceAxis() const { return m_sliceAxis; }
  void setSliceAxis(NeuTube::EAxis axis) { m_sliceAxis = axis; }

public:
  static bool isEmptyTree(const ZStackObject *obj);
  static bool isSameSource(const std::string &s1, const std::string &s2);
  static bool isSameClass(const std::string &s1, const std::string &s2);
  static bool isSelected(const ZStackObject *obj);
  template <typename T>
  static T* CastVoidPointer(void *p);

protected:
  bool m_selected;
  bool m_isSelectable;
  bool m_isVisible;
  bool m_isHittable;
  bool m_projectionVisible;
  EDisplayStyle m_style;
  QColor m_color;
  ETarget m_target;
  static double m_defaultPenWidth;
  bool m_usingCosmeticPen;
  double m_zScale;
  std::string m_source;
  std::string m_objectClass;
  std::string m_objectId;
  int m_zOrder;
  EType m_type;
  ZStackObjectRole m_role;
  ZIntPoint m_hitPoint;
  NeuTube::EAxis m_sliceAxis;

  NeuTube::Display::TVisualEffect m_visualEffect;

  mutable int m_prevDisplaySlice;
//  static const char *m_nodeAdapterId;
};

typedef ZSharedPointer<ZStackObject> ZStackObjectPtr;

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
