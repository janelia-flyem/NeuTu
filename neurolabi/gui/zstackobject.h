#ifndef ZSTACKOBJECT_H
#define ZSTACKOBJECT_H

#include "zqtheader.h"
#include "zpainter.h"

class ZStackObject
{
public:
  ZStackObject();
  virtual ~ZStackObject() {}

  virtual const std::string& className() const = 0;

  void setSelected(bool selected) { m_selected = selected; }
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

protected:
  bool m_selected;
  bool m_isVisible;
  Display_Style m_style;
  QColor m_color;
  ETarget m_target;
  static double m_defaultPenWidth;
  bool m_usingCosmeticPen;
};

#define ZSTACKOBJECT_DEFINE_CLASS_NAME(c) \
  const std::string& c::className() const {\
    static const std::string name = #c;\
    return name; \
  }

#endif // ZSTACKOBJECT_H
