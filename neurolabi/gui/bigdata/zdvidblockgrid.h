#ifndef ZDVIDBLOCKGRID_H
#define ZDVIDBLOCKGRID_H

#include "zintpoint.h"
#include "zintcuboid.h"

/*!
 * \brief The class of block grid
 *
 * The class assumes there are no more than 2G blocks in a grid. Each point on
 * the grid represents a block and all the blocks in the same grid have the same
 * size. A grid has two types of size: spatial size and grid point count. The
 * minimal point of the grid
 */
class ZDvidBlockGrid {

public:
  ZDvidBlockGrid();
  virtual ~ZDvidBlockGrid() {}

public:
  class Location {
  public:
    inline const ZIntPoint& getBlockIndex() const { return m_blockIndex; }
    inline const ZIntPoint& getLocalPosition() const { return m_localPosition; }
    inline void setBlockIndex(int x, int y, int z) {
      m_blockIndex.set(x, y, z);
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

  /*!
   * \brief Set the start index
   *
   * This function does not change the end index
   */
  void setStartIndex(int x, int y, int z);

  /*!
   * \brief Set the end index
   *
   * This function does not change the start index
   */
  void setEndIndex(int x, int y, int z);

  void setMinPoint(int x, int y, int z);
  void setBlockSize(int x, int y, int z);
  void setGridSize(int width, int height, int depth);

  ZIntPoint getBlockPosition(const ZIntPoint &blockIndex) const;
  ZIntCuboid getBlockBox(const ZIntPoint &blockIndex) const;

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

private:
  ZIntCuboid m_grid;
  ZIntPoint m_minPoint;
  ZIntPoint m_blockSize;
};

#endif

