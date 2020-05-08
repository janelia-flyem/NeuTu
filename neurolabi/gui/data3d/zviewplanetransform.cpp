#include "zviewplanetransform.h"

//#include <QTransform>

ZViewPlaneTransform::ZViewPlaneTransform()
{

}

void ZViewPlaneTransform::set(double dx, double dy, double s)
{
  m_dx = dx;
  m_dy = dy;
  m_s = s;
}

/*
QTransform ZViewPlaneTransform::getPainterTransform() const
{
  QTransform transform;
  transform.setMatrix(m_s, 0, 0, 0, m_s, 0, m_dx, m_dy, 1);

  return  transform;
}
*/

double ZViewPlaneTransform::getTx() const
{
  return m_dx;
}

double ZViewPlaneTransform::getTy() const
{
  return m_dy;
}

double ZViewPlaneTransform::getScale() const
{
  return m_s;
}

