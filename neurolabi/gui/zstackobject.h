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
 * for special purposes.
 */
class ZStackObject
{
public:
  ZStackObject();
  virtual ~ZStackObject() {}

  virtual const std::string& className() const = 0;

  enum EType {
    TYPE_SWC, TYPE_PUNCTUM, TYPE_OBJ3D,
    TYPE_STROKE, TYPE_LOCSEG_CHAIN, TYPE_CONN,
    TYPE_SPARSE_OBJECT, TYPE_CIRCLE, TYPE_STACK_BALL, TYPE_UNIDENTIFIED
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
    STACK, OBJECT_CANVAS, WIDGET
  };

  // Display an object to widget, xoffset and yoffset is top left corner of widget
  // zoom ratio is ratio of widget pixel to object pixel
  virtual void display(
      ZPainter &painter, int z = 0, Display_Style option = NORMAL) const = 0;

  /* For special painting when ZPainter cannot be created */
  virtual void display(
      QPainter *painter, int z = 0, Display_Style option = NORMAL) const;

  inline bool isVisible() const { return m_isVisible; }
  inline void setVisible(bool visible) { m_isVisible = visible; }
  inline void setDisplayStyle(Display_Style style) { m_style = style; }
  inline Display_Style displayStyle() const { return m_style; }

  inline ETarget getTarget() const { return m_target; }
  inline void setTarget(ETarget target) { m_target = target; }

  virtual bool isSliceVisible(int z) const;

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

  inline void setZScale(double scale) { m_zScale = scale; }

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

protected:
  bool m_selected;
  bool m_isVisible;
  Display_Style m_style;
  QColor m_color;
  ETarget m_target;
  static double m_defaultPenWidth;
  bool m_usingCosmeticPen;
  double m_zScale;
  std::string m_source;
  int m_zOrder;
  EType m_type;
};

#define ZSTACKOBJECT_DEFINE_CLASS_NAME(c) \
  const std::string& c::className() const {\
    static const std::string name = #c;\
    return name; \
  }

#endif // ZSTACKOBJECT_H
