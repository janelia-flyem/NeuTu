#include "zdvidblockgrid.h"
#include "tz_utilities.h"

ZDvidBlockGrid::ZDvidBlockGrid()
{

}

void ZDvidBlockGrid::setStartIndex(int x, int y, int z)
{
  m_grid.setFirstCorner(x, y, z);
}

void ZDvidBlockGrid::setEndIndex(int x, int y, int z)
{
  m_grid.setLastCorner(x, y, z);
}

void ZDvidBlockGrid::setGridSize(int width, int height, int depth)
{
  m_grid.setSize(width, height, depth);
}

void ZDvidBlockGrid::setMinPoint(int x, int y, int z)
{
  m_minPoint.set(x, y, z);
}

void ZDvidBlockGrid::setBlockSize(int x, int y, int z)
{
  m_blockSize.set(x, y, z);
}

ZIntPoint ZDvidBlockGrid::getBlockPosition(const ZIntPoint &blockIndex) const
{
  return ZIntPoint(blockIndex.getX() * m_blockSize.getX() + m_minPoint.getX(),
                   blockIndex.getY() * m_blockSize.getY() + m_minPoint.getY(),
                   blockIndex.getZ() * m_blockSize.getZ() + m_minPoint.getZ());
}

int ZDvidBlockGrid::getHashIndex(const ZIntPoint &blockIndex) const
{
  int index = -1;

  if (IS_IN_CLOSE_RANGE(blockIndex.getX(), m_grid.getFirstCorner().getX(),
                        m_grid.getLastCorner().getX()) &&
      IS_IN_CLOSE_RANGE(blockIndex.getY(), m_grid.getFirstCorner().getY(),
                        m_grid.getLastCorner().getY()) &&
      IS_IN_CLOSE_RANGE(blockIndex.getZ(), m_grid.getFirstCorner().getY(),
                        m_grid.getLastCorner().getY())) {
    ZIntPoint adjustedBlockIndex = blockIndex - m_grid.getFirstCorner();
    int area = m_grid.getWidth() * m_grid.getHeight();
    int width = m_grid.getWidth();
    index = area * adjustedBlockIndex.getZ() +
        width * adjustedBlockIndex.getY() + adjustedBlockIndex.getX();
  }

  return index;
}

ZIntCuboid ZDvidBlockGrid::getBlockBox(const ZIntPoint &blockIndex) const
{
  ZIntCuboid cuboid;
  cuboid.setFirstCorner(getBlockPosition(blockIndex));
  cuboid.setSize(m_blockSize.getX(), m_blockSize.getY(), m_blockSize.getZ());

  return cuboid;
}

int ZDvidBlockGrid::getBlockNumber() const
{
  return m_grid.getVolume();
}

bool ZDvidBlockGrid::isEmpty() const
{
  return m_blockSize.getX() == 0 || m_blockSize.getY() == 0 ||
      m_blockSize.getZ() == 0 || getBlockNumber() == 0;
}

int ZDvidBlockGrid::getSpatialDepth() const
{
  return m_blockSize.getZ() * m_grid.getDepth();
}

int ZDvidBlockGrid::getSpatialWidth() const
{
  return m_blockSize.getX() * m_grid.getWidth();
}

int ZDvidBlockGrid::getSpatialHeight() const
{
  return m_blockSize.getY() * m_grid.getHeight();
}

/**********************ZDvidBlockGrid::Location**********************/
ZDvidBlockGrid::Location ZDvidBlockGrid::getLocation(int x, int y, int z) const
{
  ZDvidBlockGrid::Location location;

  x -= m_minPoint.getX();
  y -= m_minPoint.getY();
  z -= m_minPoint.getZ();

  location.setBlockIndex(x / m_blockSize.getX() + m_grid.getFirstCorner().getX(),
                         y / m_blockSize.getY() + m_grid.getFirstCorner().getY(),
                         z / m_blockSize.getZ() + m_grid.getFirstCorner().getZ());

  ZIntPoint blockPosition = getBlockPosition(location.getBlockIndex());

  location.setLocalPosition(x - blockPosition.getX(),
                            y - blockPosition.getY(),
                            z - blockPosition.getZ());

  return location;
}
