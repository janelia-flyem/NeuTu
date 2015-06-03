#include "zblockgrid.h"
#include "tz_utilities.h"

ZBlockGrid::ZBlockGrid()
{
}

void ZBlockGrid::setGridSize(int width, int height, int depth)
{
  m_size.set(width, height, depth);
}

void ZBlockGrid::setGridSize(const ZIntPoint &s)
{
  setGridSize(s.getX(), s.getY(), s.getZ());
}

void ZBlockGrid::setMinPoint(int x, int y, int z)
{
  m_minPoint.set(x, y, z);
}

void ZBlockGrid::setMinPoint(const ZIntPoint &pt)
{
  setMinPoint(pt.getX(), pt.getY(), pt.getZ());
}

void ZBlockGrid::setBlockSize(int x, int y, int z)
{
  m_blockSize.set(x, y, z);
}

void ZBlockGrid::setBlockSize(const ZIntPoint &s)
{
  setBlockSize(s.getX(), s.getY(), s.getZ());
}

ZIntPoint ZBlockGrid::getBlockPosition(const ZIntPoint &blockIndex) const
{
  int x = blockIndex.getX() * m_blockSize.getX();
  int y = blockIndex.getY() * m_blockSize.getY();
  int z = blockIndex.getZ() * m_blockSize.getZ();

  x += m_minPoint.getX();
  y += m_minPoint.getY();
  z += m_minPoint.getZ();

  return ZIntPoint(x, y, z);
}

int ZBlockGrid::getHashIndex(const ZIntPoint &blockIndex) const
{
  int index = -1;

  if (IS_IN_OPEN_RANGE(blockIndex.getX(), -1, m_size.getX()) &&
      IS_IN_OPEN_RANGE(blockIndex.getY(), -1, m_size.getY()) &&
      IS_IN_OPEN_RANGE(blockIndex.getZ(), -1, m_size.getZ())) {
    //ZIntPoint adjustedBlockIndex = blockIndex - m_grid.getFirstCorner();
    int area = m_size.getX() * m_size.getY();
    int width = m_size.getX();
    index = area * blockIndex.getZ() + width * blockIndex.getY() +
        blockIndex.getX();
  }

  return index;
}

ZIntCuboid ZBlockGrid::getBlockBox(const ZIntPoint &blockIndex) const
{
  ZIntCuboid cuboid;
  cuboid.setFirstCorner(getBlockPosition(blockIndex));
  cuboid.setSize(m_blockSize.getX(), m_blockSize.getY(), m_blockSize.getZ());

  return cuboid;
}

int ZBlockGrid::getBlockNumber() const
{
  return m_size.getX() * m_size.getY() * m_size.getZ();
}

bool ZBlockGrid::isEmpty() const
{
  return m_size.getX() == 0 || m_size.getY() == 0 || m_size.getZ() == 0 ||
      m_blockSize.getX() == 0 || m_blockSize.getY() == 0 ||
      m_blockSize.getZ() == 0;
}

int ZBlockGrid::getSpatialDepth() const
{
  return m_blockSize.getZ() * m_size.getZ();
}

int ZBlockGrid::getSpatialWidth() const
{
  return m_blockSize.getX() * m_size.getX();
}

int ZBlockGrid::getSpatialHeight() const
{
  return m_blockSize.getY() * m_size.getY();
}

ZIntPoint ZBlockGrid::getBlockIndex(int x, int y, int z) const
{
  return getLocation(x, y, z).getBlockIndex();
}

/**********************ZDvidBlockGrid::Location**********************/
ZBlockGrid::Location ZBlockGrid::getLocation(int x, int y, int z) const
{
  ZBlockGrid::Location location;

  x -= m_minPoint.getX();
  y -= m_minPoint.getY();
  z -= m_minPoint.getZ();

  ZIntPoint blockIndex;
  if (x >= 0) {
    blockIndex.setX(x / m_blockSize.getX());
  } else {
    blockIndex.setX(x / m_blockSize.getX() - 1);
  }

  if (y >= 0) {
    blockIndex.setY(y / m_blockSize.getY());
  } else {
    blockIndex.setY(y / m_blockSize.getY() - 1);
  }

  if (z >= 0) {
    blockIndex.setZ(z / m_blockSize.getZ());
  } else {
    blockIndex.setZ(z / m_blockSize.getZ() - 1);
  }

  location.setBlockIndex(blockIndex);

  location.setLocalPosition(x - blockIndex.getX() * m_blockSize.getX(),
                            y - blockIndex.getY() * m_blockSize.getY(),
                            z - blockIndex.getZ() * m_blockSize.getZ());

  return location;
}
