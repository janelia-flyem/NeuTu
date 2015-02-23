#include "zrect2d.h"

#include <QPainter>

#include "zpainter.h"
#include "tz_math.h"

ZRect2d::ZRect2d() : m_x0(0), m_y0(0), m_width(0), m_height(0), m_z(0),
  m_isPenetrating(false)
{
  m_type = ZStackObject::TYPE_RECT2D;
}

ZRect2d::ZRect2d(int x0, int y0, int width, int height) :
  m_x0(x0), m_y0(y0), m_width(width), m_height(height), m_z(0),
  m_isPenetrating(false)
{
  m_type = ZStackObject::TYPE_RECT2D;
}

void ZRect2d::set(int x0, int y0, int width, int height)
{
  m_x0 = x0;
  m_y0 = y0;
  m_width = width;
  m_height = height;
}

bool ZRect2d::isValid() const
{
  return m_width > 0 && m_height > 0;
}

bool ZRect2d::isSliceVisible(int z) const
{
  return isValid() && (m_isPenetrating || z == m_z);
}

void ZRect2d::display(ZPainter &painter, int slice, EDisplayStyle /*option*/) const
{
  int z = slice + iround(painter.getOffset().z());
  if (!(isSliceVisible(z) || (slice == -1))) {
    return;
  }

  QColor color = m_color;
  QPen pen(color);
  if (isSelected()) {
    pen.setWidth(pen.width() + 5);
    pen.setStyle(Qt::DashLine);
  }

  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);



  painter.drawRect(m_x0, m_y0, m_width, m_height);
}

void ZRect2d::setLastCorner(int x, int y)
{
  m_width = x - m_x0 + 1;
  m_height = y - m_y0 + 1;
}

void ZRect2d::setFirstCorner(int x, int y)
{
  m_x0 = x;
  m_y0 = y;
}

bool ZRect2d::hit(double x, double y)
{
  return ((x >= m_x0 - 5 && y >= m_y0 - 5 &&
       x < m_x0 + m_width + 5 && y < m_y0 + m_height + 5) &&
      !(x >= m_x0 + 5 && y >= m_y0 + 5 &&
        x < m_x0 + m_width - 5 && y < m_y0 + m_height - 5));
}

bool ZRect2d::hit(double x, double y, double z)
{
  if (m_isPenetrating) {
    return hit(x, y);
  }

  if (iround(z) == m_z) {
    return ((x >= m_x0 - 5 && y >= m_y0 - 5 &&
             x < m_x0 + m_width + 5 && y < m_y0 + m_height + 5) &&
            !(x >= m_x0 + 5 && y >= m_y0 + 5 &&
              x < m_x0 + m_width - 5 && y < m_y0 + m_height - 5));
  }

  return false;
}


ZSTACKOBJECT_DEFINE_CLASS_NAME(ZRect2d)
