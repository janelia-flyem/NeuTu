#include "zintcuboid.h"
#include "tz_utilities.h"

ZIntCuboid::ZIntCuboid() : m_lastCorner(-1, -1, -1)
{
}

ZIntCuboid::ZIntCuboid(int x1, int y1, int z1, int x2, int y2, int z2)
{
  m_firstCorner.set(x1, y1, z1);
  m_lastCorner.set(x2, y2, z2);
}

int ZIntCuboid::getWidth() const
{
  return m_lastCorner.getX() - m_firstCorner.getX() + 1;
}

int ZIntCuboid::getHeight() const
{
  return m_lastCorner.getY() - m_firstCorner.getY() + 1;
}

int ZIntCuboid::getDepth() const
{
  return m_lastCorner.getZ() - m_firstCorner.getZ() + 1;
}

void ZIntCuboid::setSize(int width, int height, int depth)
{
  m_lastCorner.set(m_firstCorner.getX() + width - 1,
                   m_firstCorner.getY() + height - 1,
                   m_firstCorner.getZ() + depth - 1);
}

void ZIntCuboid::set(int x1, int y1, int z1, int x2, int y2, int z2)
{
  setFirstCorner(x1, y1, z1);
  setLastCorner(x2, y2, z2);
}

void ZIntCuboid::join(const ZIntCuboid &cuboid)
{
  for (int i = 0; i < 3; i++) {
    m_firstCorner[i] = imin2(m_firstCorner[i], cuboid.m_firstCorner[i]);
    m_lastCorner[i] = imax2(m_lastCorner[i], cuboid.m_lastCorner[i]);
  }
}

void ZIntCuboid::joinX(int x)
{
  if (x < m_firstCorner.getX()) {
    m_firstCorner.setX(x);
  } else if (x > m_lastCorner.getX()) {
    m_lastCorner.setX(x);
  }
}

void ZIntCuboid::joinY(int y)
{
  if (y < m_firstCorner.getY()) {
    m_firstCorner.setY(y);
  } else if (y > m_lastCorner.getY()) {
    m_lastCorner.setY(y);
  }
}

void ZIntCuboid::joinZ(int z)
{
  if (z < m_firstCorner.getZ()) {
    m_firstCorner.setZ(z);
  } else if (z > m_lastCorner.getZ()) {
    m_lastCorner.setZ(z);
  }
}

size_t ZIntCuboid::getVolume() const
{
  if (getWidth() <= 0 || getHeight() <= 0 || getDepth() <= 0) {
    return 0;
  }

  size_t area = getWidth() * getHeight();

  return area * getDepth();
}

bool ZIntCuboid::contains(int x, int y, int z) const
{
  return IS_IN_CLOSE_RANGE(x, m_firstCorner.getX(), m_lastCorner.getX()) &&
      IS_IN_CLOSE_RANGE(y, m_firstCorner.getY(), m_lastCorner.getY()) &&
      IS_IN_CLOSE_RANGE(z, m_firstCorner.getZ(), m_lastCorner.getZ());
}

bool ZIntCuboid::isEmpty() const
{
  return getWidth() <= 0 || getHeight() <= 0 || getDepth() <= 0;
}

void ZIntCuboid::setFirstX(int x)
{
  m_firstCorner.setX(x);
}

void ZIntCuboid::setLastX(int x)
{
  m_lastCorner.setX(x);
}

void ZIntCuboid::setFirstY(int y)
{
  m_firstCorner.setY(y);
}

void ZIntCuboid::setLastY(int y)
{
  m_lastCorner.setY(y);
}
