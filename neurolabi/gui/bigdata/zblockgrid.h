#ifndef ZBLOCKGRID_H
#define ZBLOCKGRID_H

#include "zintpoint.h"
#include "zintcuboid.h"

/*!
 * \brief The class of block grid
 *
 * The class assumes there are no more than 2G blocks in a grid. Each point on
 * the grid represents a block and all the blocks in the same grid have the same
 * size. A grid has two types of size: spatial size and grid point count. The
 * minimal point of the grid is the coordinates of the first corner of the first
 * block.
 */
class ZBlockGrid
{
public:
  ZBlockGrid();
  virtual ~ZBlockGrid() {}

public:
  class Location {
  public:
    inline const ZIntPoint& getBlockIndex() const { return m_blockIndex; }
    inline const ZIntPoint& getLocalPosition() const { return m_localPosition; }

    inline void setBlockIndex(int x, int y, int z) {
      m_blockIndex.set(x, y, z);
    }
    inline void setBlockIndex(const ZIntPoint &blockIndex) {
      m_blockIndex = blockIndex;
    }

    inline void setLocalPosition(int x, int y, int z) {
      m_localPosition.set(x, y, z);
    }

  private:
    ZIntPoint m_blockIndex;
    ZIntPoint m_localPosition;
  };

  /*!
   * \brief Get the hash index of a block index
   *
   * \return -1 the index if out of range.
   */
  int getHashIndex(const ZIntPoint &blockIndex) const;

  /*!
   * \brief Get the grid location of a global point
   */
  Location getLocation(int x, int y, int z) const;

  ZIntPoint getBlockIndex(int x, int y, int z) const;

  void setMinPoint(int x, int y, int z);
  void setMinPoint(const ZIntPoint &pt);
  void setBlockSize(int x, int y, int z);
  void setBlockSize(const ZIntPoint &s);
  void setGridSize(int width, int height, int depth);
  void setGridSize(const ZIntPoint &s);

  ZIntPoint getBlockPosition(const ZIntPoint &blockIndex) const;
  ZIntCuboid getBlockBox(const ZIntPoint &blockIndex) const;

  inline const ZIntPoint& getBlockSize() const { return m_blockSize; }
  inline const ZIntPoint& getGridSize() const { return m_size; }
  inline const ZIntPoint& getMinPoint() const { return m_minPoint; }

  /*!
   * \brief Check if a grid is empty.
   *
   * A grid is empty if the block size is 0 in any dimension or the block
   * number is 0.
   */
  bool isEmpty() const;

  int getBlockNumber() const;

  /*!
   * \brief Spatial width of the grid.
   */
  int getSpatialWidth() const;
  int getSpatialHeight() const;
  int getSpatialDepth() const;

protected:
  ZIntPoint m_size; //Grid size
  ZIntPoint m_minPoint;
  ZIntPoint m_blockSize;
};

#endif // ZBLOCKGRID_H
