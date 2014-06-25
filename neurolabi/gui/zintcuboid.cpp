#include "zintcuboid.h"

ZIntCuboid::ZIntCuboid()
{
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
