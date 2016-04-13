/**@file zellipse.h
 * @brief Ellipse class
 * @author Ting Zhao
 * @date 01-JUN-2009
 */

#ifndef _ZELLIPSE_H_
#define _ZELLIPSE_H_

#if defined(_QT_GUI_USED_)
#include <QPointF>
#endif

#include "zstackobject.h"

class ZEllipse : public ZStackObject {
public:
  ZEllipse(const QPointF &center, double rx, double ry);

  virtual const std::string& className() const;

public:
  using ZStackObject::display; // suppress warning: hides overloaded virtual function [-Woverloaded-virtual]
  void display(ZPainter &painter, int z = 0, EDisplayStyle option = NORMAL)
  const;

  virtual void save(const char *filePath);
  virtual bool load(const char *filePath);

  inline void setAngle(double angle) {
    m_angle = angle;
  }

private:
  QPointF m_center;
  double m_rx;
  double m_ry;
  double m_angle; //angle of m_rx to the X axis
};

#endif /* _ZELLIPSE_H_ */
