#include "zblockgrid.h"

#include <queue>
#include <unordered_map>

#include "geometry/zgeometry.h"
#include "geometry/zaffinerect.h"

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

void ZBlockGrid::configure(const ZBlockGrid &grid, int zoom)
{
  int scale = zgeom::GetZoomScale(zoom);
  m_minPoint = grid.m_minPoint / scale;
  m_blockSize = grid.m_blockSize;
  m_size = grid.m_size / scale;
  if (scale > 1) {
    m_size = m_size + 1;
  }
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
  cuboid.setMinCorner(getBlockPosition(blockIndex));
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

ZIntPoint ZBlockGrid::getBlockIndex(const ZIntPoint &pos) const
{
  return getBlockIndex(pos.getX(), pos.getY(), pos.getZ());
}

bool ZBlockGrid::containsBlock(int i, int j, int k) const
{
  return (i >= 0) && (j >= 0) && (k >= 0) && (i < m_size.getX()) &&
      (j < m_size.getY()) && (k < m_size.getZ());
}

bool ZBlockGrid::containsBlock(const ZIntPoint &index) const
{
  return containsBlock(index.getX(), index.getY(), index.getZ());
}

void ZBlockGrid::forEachIntersectedBlock(
    const ZAffineRect &plane, std::function<void(int i, int j, int k)> f)
{
  std::queue<ZIntPoint> blockQueue;
  ZIntPoint seedBlock = getBlockIndex(plane.getCenter().toIntPoint());
  blockQueue.push(seedBlock);
  std::unordered_map<std::string, bool> checked;
  while (!blockQueue.empty()) {
    ZIntPoint block = blockQueue.front();
    if (zgeom::Intersects(plane, getBlockBox(block))) {
      if (containsBlock(block)) {
        f(block.getX(), block.getY(), block.getZ());
      }
      zgeom::raster::ForEachNeighbor<3>(
            block.getX(), block.getY(), block.getZ(), [&](int x, int y, int z) {
        std::string key =
            std::to_string(x) + "_" + std::to_string(y) + "_" + std::to_string(z);
        if (!checked[key]) {
          if (zgeom::Intersects(plane, getBlockBox(ZIntPoint(x, y, z)))) {
            blockQueue.push(ZIntPoint(x, y, z));
            checked[key] = true;
          }
        }
      });
    }
    blockQueue.pop();
  }
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
