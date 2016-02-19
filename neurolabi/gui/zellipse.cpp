/*
 * zellipse.cpp
 *
 *  Created on: Jun 1, 2009
 *      Author: zhaot
 */

#if _QT_GUI_USED_
#include <QtGui>
#endif
#include "tz_cdefs.h"
#include "zellipse.h"
#include "zpainter.h"

ZEllipse::ZEllipse(const QPointF &center, double rx, double ry) : m_angle(0)
{
  m_center = center;
  m_rx = rx;
  m_ry = ry;
}

void ZEllipse::display(ZPainter &painter, int z, EDisplayStyle option) const
{
  UNUSED_PARAMETER(z);
  UNUSED_PARAMETER(option);

#if _QT_GUI_USED_
  //painter.setPen(QPen(QColor(255, 0, 0, 32), .7));
  painter.setPen(getColor());

  QTransform oldTransform = painter.getTransform();

  QTransform transform;
  //transform.rotate(m_angle);
  transform.translate(m_center.x(), m_center.y());
  transform.rotate(m_angle);


  painter.setTransform(transform, true);
  //painter.setTransform(oldTransform, true);

  painter.drawEllipse(QPointF(0, 0), m_rx, m_ry);

  painter.setTransform(oldTransform);

//  return true;
#endif
}

void ZEllipse::save(const char *filePath)
{
  UNUSED_PARAMETER(filePath);
}

bool ZEllipse::load(const char *filePath)
{
  UNUSED_PARAMETER(filePath);

  return false;
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZEllipse)
