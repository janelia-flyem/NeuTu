#ifndef ZSTACKOBJECT_H
#define ZSTACKOBJECT_H

#include "zqtheader.h"
#include "zpainter.h"

/*!
 * \brief The abstract class of representing an 3D object
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
 */
class ZStackObject
{
public:
  ZStackObject();
  virtual ~ZStackObject() {}

  virtual const std::string& className() const = 0;

  enum EType {
    TYPE_UNIDENTIFIED = 0,
    TYPE_SWC, TYPE_PUNCTUM, TYPE_OBJ3D,
    TYPE_STROKE, TYPE_LOCSEG_CHAIN, TYPE_CONN,
    TYPE_OBJECT3D_SCAN,
    TYPE_SPARSE_OBJECT, TYPE_CIRCLE, TYPE_STACK_BALL,
    TYPE_STACK_PATCH, TYPE_RECT2D
  };

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

  enum Palette_Color {
    BLUE = 0, GREEN, RED, ALPHA
  };

  enum Display_Style {
    NORMAL, SOLID, BOUNDARY, SKELETON
  };

  enum ETarget {
    STACK_CANVAS, OBJECT_CANVAS, WIDGET
  };

  enum EDisplaySliceMode {
    DISPLAY_SLICE_PROJECTION, DISPLAY_SLICE_SINGLE
  };

  // Display an object to widget, xoffset and yoffset is top left corner of widget
  // zoom ratio is ratio of widget pixel to object pixel
  virtual void display(
      ZPainter &painter, int slice, Display_Style option) const = 0;

  /* For special painting when ZPainter cannot be created */
  virtual void display(
      QPainter *painter, int z, Display_Style option,
      EDisplaySliceMode sliceMode) const;

  inline bool isVisible() const { return m_isVisible; }
  inline void setVisible(bool visible) { m_isVisible = visible; }
  inline void setDisplayStyle(Display_Style style) { m_style = style; }
  inline Display_Style displayStyle() const { return m_style; }

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
  Display_Style m_style;
  QColor m_color;
  ETarget m_target;
  static double m_defaultPenWidth;
  bool m_usingCosmeticPen;
  double m_zScale;
  std::string m_source;
  int m_zOrder;
  EType m_type;

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
