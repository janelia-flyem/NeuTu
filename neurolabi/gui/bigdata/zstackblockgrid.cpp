#include "zstackblockgrid.h"
#include "zstack.hxx"
#include "zintcuboid.h"

ZStackBlockGrid::ZStackBlockGrid()
{
}

ZStackBlockGrid::~ZStackBlockGrid()
{
  clearStack();
}

void ZStackBlockGrid::clearStack()
{
  for (std::vector<ZStack*>::iterator iter = m_stackArray.begin();
       iter != m_stackArray.end(); ++iter) {
    delete *iter;
  }
  m_stackArray.clear();
}

bool ZStackBlockGrid::consumeStack(const ZIntPoint &blockIndex, ZStack *stack)
{
  int index = getHashIndex(blockIndex);
  if (index < 0) {
    delete stack;
    return false;
  }

  if (index >= (int) m_stackArray.size()) {
    m_stackArray.resize(index + 1, NULL);
  } else {
    ZStack *oldStack = m_stackArray[index];
    if (oldStack != NULL && oldStack != stack) {
      delete oldStack;
    }
  }

  stack->setOffset(getBlockPosition(blockIndex));

  m_stackArray[index] = stack;

  return true;
}

ZStack* ZStackBlockGrid::getStack(const ZIntPoint &blockIndex) const
{
  ZStack *stack = NULL;

  int index = getHashIndex(blockIndex);
  if (index >= 0 && index < (int) m_stackArray.size()) {
    stack = m_stackArray[index];
  }

  return stack;
}

int ZStackBlockGrid::getValue(int x, int y, int z) const
{
  Location location = getLocation(x, y, z);
  ZStack *stack = getStack(location.getBlockIndex());
  int v = 0;
  if (stack != NULL) {
    v = stack->getIntValueLocal(location.getLocalPosition().getX(),
                                location.getLocalPosition().getY(),
                                location.getLocalPosition().getZ());
  }

  return v;
}

ZStack* ZStackBlockGrid::toStack() const
{
  if (isEmpty()) {
    return NULL;
  }

  ZIntCuboid box = getStackBoundBox();

  //int width = getSpatialWidth();
  //int height = getSpatialHeight();
  //int depth = getSpatialDepth();

  ZStack *out =
      new ZStack(GREY, box.getWidth(), box.getHeight(), box.getDepth(), 1);
  out->setOffset(box.getFirstCorner());
  out->setZero();

  for (int z = 0; z < m_size.getZ(); ++z) {
    for (int y = 0; y < m_size.getY(); ++y) {
      for (int x = 0; x < m_size.getX(); ++x) {
        ZStack *stack = getStack(ZIntPoint(x, y, z));
        if (stack != NULL) {
          ZIntCuboid box = getBlockBox(ZIntPoint(x, y, z));
          out->setBlockValue(box.getFirstCorner().getX(),
                             box.getFirstCorner().getY(),
                             box.getFirstCorner().getZ(), stack);
        }
      }
    }
  }

  return out;
}

void ZStackBlockGrid::downsampleBlock(int xintv, int yintv, int zintv)
{
  m_size.set(m_size.getX() / (xintv + 1), m_size.getY() / (yintv + 1),
             m_size.getX() / (zintv + 1));
  if (isEmpty()) {
    clearStack();
  } else {
    for (std::vector<ZStack*>::iterator iter = m_stackArray.begin();
         iter != m_stackArray.end(); ++iter) {
      ZStack *stack = *iter;
      stack->downsampleMax(xintv, yintv, zintv);
    }
  }
}

ZStackBlockGrid* ZStackBlockGrid::makeDownsample(int xintv, int yintv, int zintv)
{
  ZStackBlockGrid *grid = new ZStackBlockGrid;
  grid->setBlockSize(
        getBlockSize() / ZIntPoint(xintv + 1, yintv + 1, zintv + 1));
  grid->setGridSize(getGridSize());
  grid->setMinPoint(getMinPoint() / ZIntPoint(xintv + 1, yintv + 1, zintv + 1));

  if (isEmpty()) {
    clearStack();
  } else {
    grid->m_stackArray.resize(m_stackArray.size(), NULL);
    for (size_t i = 0; i < m_stackArray.size(); ++i) {
      ZStack *stack = m_stackArray[i];
      if (stack != NULL) {
        ZStack *dsStack = stack->clone();
        dsStack->downsampleMax(xintv, yintv, zintv);
        grid->m_stackArray[i] = dsStack;
      }
    }
  }

  return grid;
}

ZIntCuboid ZStackBlockGrid::getStackBoundBox() const
{
  ZIntCuboid cuboid;
  bool isInitialized = false;
  for (std::vector<ZStack*>::const_iterator iter = m_stackArray.begin();
       iter != m_stackArray.end(); ++iter) {
    const ZStack *stack = *iter;
    if (stack != NULL) {
      if (isInitialized) {
        cuboid.join(stack->getBoundBox());
      } else {
        cuboid = stack->getBoundBox();
        isInitialized = true;
      }
    }
  }

  return cuboid;
}
