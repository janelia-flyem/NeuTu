#include "zintcuboidobj.h"

#include <QColor>
#include <QPen>
#include <QBrush>

#include "zpainter.h"
#include "zintpoint.h"


ZIntCuboidObj::ZIntCuboidObj()
{
  m_type = ZStackObject::TYPE_INT_CUBOID;
}

bool ZIntCuboidObj::isSliceVisible(int /*z*/) const
{
  return true;
}

const ZIntPoint& ZIntCuboidObj::getFirstCorner() const
{
  return m_cuboid.getFirstCorner();
}

const ZIntPoint& ZIntCuboidObj::getLastCorner() const
{
  return m_cuboid.getLastCorner();
}

void ZIntCuboidObj::setFirstCorner(const ZIntPoint &firstCorner)
{
  m_cuboid.setFirstCorner(firstCorner);
}

void ZIntCuboidObj::setLastCorner(const ZIntPoint &lastCorner)
{
  m_cuboid.setLastCorner(lastCorner);
}

void ZIntCuboidObj::setFirstCorner(int x, int y, int z)
{
  m_cuboid.setFirstCorner(x, y, z);
}

void ZIntCuboidObj::setLastCorner(int x, int y, int z)
{
  m_cuboid.setLastCorner(x, y, z);
}

int ZIntCuboidObj::getWidth() const
{
  return m_cuboid.getWidth();
}

int ZIntCuboidObj::getHeight() const
{
  return m_cuboid.getHeight();
}

int ZIntCuboidObj::getDepth() const
{
  return m_cuboid.getDepth();
}

bool ZIntCuboidObj::isOnSlice(int z) const
{
  return z >= getFirstCorner().getZ() && z <= getLastCorner().getZ();
}

void ZIntCuboidObj::display(ZPainter &painter, int slice, EDisplayStyle /*option*/) const
{
  if (m_cuboid.isEmpty()) {
    return;
  }

  int z = painter.getZ(slice);

  if (!(isSliceVisible(z) || (slice < 0))) {
    return;
  }

  QColor color = m_color;
  QPen pen(color);

  if(isOnSlice(z) || slice < 0) {
    if (isSelected()) {
      pen.setStyle(Qt::DashLine);
    } else {
      pen.setStyle(Qt::SolidLine);
    }
  } else {
    pen.setStyle(Qt::DotLine);
  }

  if (isSelected()) {
    pen.setWidth(pen.width() + 5);
  }

  painter.setPen(pen);
  painter.setBrush(Qt::NoBrush);

  painter.drawRect(getFirstCorner().getX(), getFirstCorner().getY(),
                   getWidth(), getHeight());
}

bool ZIntCuboidObj::hit(double x, double y)
{
  return ((x >= getFirstCorner().getX() - 5 && y >= getFirstCorner().getY() - 5 &&
           x < getLastCorner().getX() + 5 && y < getLastCorner().getY() + 5) &&
          !(x >= getFirstCorner().getX() + 5 && y >= getFirstCorner().getY() + 5 &&
            x < getLastCorner().getX() - 5 && y < getLastCorner().getY() - 5));
}

bool ZIntCuboidObj::hit(double x, double y, double z)
{
  if (isOnSlice(z)) {
    return hit(x, y);
  }

  return false;
}

void ZIntCuboidObj::clear()
{
  m_cuboid.set(0, 0, 0, -1, -1, -1);
}

bool ZIntCuboidObj::isValid() const
{
  return !m_cuboid.isEmpty();
}

void ZIntCuboidObj::join(const ZIntCuboid &cuboid)
{
  m_cuboid.join(cuboid);
}

ZSTACKOBJECT_DEFINE_CLASS_NAME(ZIntCuboidObj)
