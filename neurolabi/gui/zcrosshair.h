#ifndef ZCROSSHAIR_H
#define ZCROSSHAIR_H

#include "zstackobject.h"
#include "geometry/zpoint.h"

/*!
 * \brief The class of crosshair for orthogonal view
 *
 * The main attribute of a crosshair object is its center, which is defined in
 * the widget/view space.
 */
class ZCrossHair : public ZStackObject
{
public:
  ZCrossHair();

  static constexpr ZStackObject::EType GetType() {
    return ZStackObject::EType::CROSS_HAIR;
  }

//  virtual const std::string& className() const;

public:
  bool display(
      QPainter *painter, const DisplayConfig &config) const {
    return false;
  }
  /*
  virtual void display(ZPainter &painter, int slice,
                       EDisplayStyle option, neutu::EAxis sliceAxis) const;
                       */

  ZPoint getCenter() const {
    return m_center;
  }

  void setCenter(double x, double y, double z);
  void setCenter(const ZPoint &center);
  void setCenter(const ZIntPoint &center);
//  void setCenter(double x, double y);
  void setX(double x);
  void setY(double y);
  void setZ(double z);

  bool hitWidgetPos(const ZIntPoint &widgetPos, neutu::EAxis axis);


private:
  void init();

private:
  ZPoint m_center;
};

#endif // ZCROSSHAIR_H
