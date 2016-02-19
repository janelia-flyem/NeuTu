#ifndef ZSTACKBLOCKGRID_H
#define ZSTACKBLOCKGRID_H

#include <vector>

#include "zblockgrid.h"
class ZStack;

class ZStackBlockGrid : public ZBlockGrid
{
public:
  ZStackBlockGrid();
  ~ZStackBlockGrid();

  /*!
   * \brief Set a stack at a certain grid point
   *
   * \a stack is put at \a blockIndex if the index is in range. The
   * offset of \a stack will be set to the first corner of the block.
   *
   * \return true iff the stack is taken successfully.
   */
  bool consumeStack(const ZIntPoint &blockIndex, ZStack *stack);

  void consumeStack(const ZIntPoint &blockIndex, std::vector<ZStack*> stackArray);

  int getValue(int x, int y, int z) const;

  ZStack* getStack(const ZIntPoint &blockIndex) const;

  void clearStack();

  ZStack* toStack() const;

  /*!
   * \brief Get the bounding box containing all non-NULL stacks
   */
  ZIntCuboid getStackBoundBox() const;

  /*!
   * \brief Downsample the grid.
   *
   * Downsample the grid by downsampling each box.
   */
  //void downsampleBlock(int xintv, int yintv, int zintv);

  ZStackBlockGrid* makeDownsample(int xintv, int yintv, int zintv);

  inline std::vector<ZStack*>& getStackArray() {
    return m_stackArray;
  }

  inline const std::vector<ZStack*>& getStackArray() const {
    return m_stackArray;
  }

private:
  std::vector<ZStack*> m_stackArray;
};

#endif // ZSTACKBLOCKGRID_H
