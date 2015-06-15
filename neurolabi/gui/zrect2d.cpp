#include "zrect2d.h"

#include <cmath>
#include <QRect>
#include <QRectF>

#include "zpainter.h"
#include "tz_math.h"
#include "zsttransform.h"

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
  int z = slice + iround(painter.getZOffset());
  if (!(isSliceVisible(z) || (slice < 0))) {
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

int ZRect2d::getFirstX() const
{
  return m_x0;
}

int ZRect2d::getFirstY() const
{
  return m_y0;
}

int ZRect2d::getLastX() const
{
  return m_width + m_x0 - 1;
}

int ZRect2d::getLastY() const
{
  return m_height + m_y0 - 1;
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

bool ZRect2d::IsEqual(const QRect &rect1, const QRect &rect2)
{
  return (rect1.left() == rect2.left()) && (rect1.top() == rect2.top()) &&
      (rect1.right() == rect2.right()) && (rect1.top() == rect2.top());
}

bool ZRect2d::IsEqual(const QRectF &rect1, const QRectF &rect2)
{
  return (rect1.left() == rect2.left()) && (rect1.top() == rect2.top()) &&
      (rect1.right() == rect2.right()) && (rect1.top() == rect2.top());
}

QRect ZRect2d::QRectBound(const QRectF &rect)
{
  QRect out;
  out.setLeft(std::floor(rect.left()));
  out.setTop(std::floor(rect.top()));
  out.setBottom(std::ceil(rect.bottom()));
  out.setRight(std::ceil(rect.right()));

  return out;
}

QRectF ZRect2d::CropRect(
    const QRectF &sourceRectIn, const QRectF &sourceRectOut,
    const QRectF &targetRectIn)
{
  if (IsEqual(sourceRectIn, sourceRectOut)) {
    return targetRectIn;
  }

  QRectF out;
  ZStTransform transform;
  transform.estimate(sourceRectIn, sourceRectOut);
  out = transform.transform(targetRectIn);

  return out;
}


ZSTACKOBJECT_DEFINE_CLASS_NAME(ZRect2d)
