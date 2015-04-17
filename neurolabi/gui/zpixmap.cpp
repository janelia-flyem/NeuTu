#include "zpixmap.h"

ZPixmap::ZPixmap() : m_isVisible(false)
{
}

ZPixmap::ZPixmap(const QSize &size) : QPixmap(size), m_isVisible(false)
{

}

const ZStTransform& ZPixmap::getTransform() const
{
  return m_transform;
}

void ZPixmap::setScale(double sx, double sy)
{
  m_transform.setScale(sx, sy);
}

void ZPixmap::setOffset(double dx, double dy)
{
  m_transform.setOffset(dx, dy);
}

void ZPixmap::clearnUp()
{
  fill(Qt::transparent);
  m_isVisible = false;
}
