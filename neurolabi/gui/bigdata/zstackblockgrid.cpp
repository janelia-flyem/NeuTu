#include "zstackblockgrid.h"
#include "zstack.hxx"
#include "geometry/zintcuboid.h"
#include "neutubeconfig.h"
#include "common/utilities.h"

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

void ZStackBlockGrid::consumeStack(
    const ZIntPoint &blockIndex, std::vector<ZStack *> stackArray)
{
  ZIntPoint currentBlockIndex = blockIndex;
  for (size_t i = 0; i < stackArray.size(); ++i) {
    consumeStack(currentBlockIndex, stackArray[i]);
    currentBlockIndex.setX(currentBlockIndex.getX() + 1);
  }
}

bool ZStackBlockGrid::consumeStack(const ZIntPoint &blockIndex, ZStack *stack)
{
  if (stack == NULL) {
    return false;
  }

  int blockNumber = stack->width() / getBlockSize().getX();
  if (blockNumber == 0) {
    delete stack;
    return false;
  }

  if (blockNumber == 1) {
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

    //stack->setOffset(getBlockPosition(blockIndex));

    m_stackArray[index] = stack;
  } else {
#ifdef _DEBUG_2
    stack->save(GET_DATA_DIR + "/test.tif");
#endif

    ZIntPoint currentBlockIndex = blockIndex;
    for (int i = 0; i < blockNumber; ++i) {
      int index = getHashIndex(currentBlockIndex);
      if (index >= 0) {
        ZIntCuboid box = getBlockBox(currentBlockIndex);
        consumeStack(currentBlockIndex, stack->makeCrop(box));
      }
      currentBlockIndex.setX(currentBlockIndex.getX() + 1);
    }

    delete stack;
  }

  return true;
}

void ZStackBlockGrid::consume(ZStackBlockGrid &grid)
{
  bool compatible = true;

  //Check compatibility
  if (m_blockSize != grid.m_blockSize || m_minPoint != grid.m_minPoint) {
    compatible = false;
  }

  if (compatible) {
    if (grid.m_stackArray.size() > m_stackArray.size()) {
      m_stackArray.resize(grid.m_stackArray.size(), NULL);
      for (size_t i = 0; i < m_stackArray.size(); ++i) {
        if (m_stackArray[i] == NULL && grid.m_stackArray[i] != NULL) {
          m_stackArray[i] = grid.m_stackArray[i];
          grid.m_stackArray[i] = NULL;
        }
      }

      for (int i = 0; i < 3; ++i) {
        if (m_size[i] < grid.m_size[i]) {
          m_size[i] = grid.m_size[i];
        }
      }
    }
  }
}

void ZStackBlockGrid::consume(ZStackBlockGrid *&grid)
{
  if (grid != NULL) {
    consume(*grid);
    delete grid;
    grid = NULL;
  }
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


bool ZStackBlockGrid::hasStack(int x,int y,int z) const
{
  Location location = getLocation(x, y, z);
  int index = getHashIndex(location.getBlockIndex());
  return index>=0 && index < (int) m_stackArray.size();
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
      new ZStack(GREY, box, 1);
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

/*
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
*/

ZStackBlockGrid* ZStackBlockGrid::makeDownsample(
    int xintv, int yintv, int zintv) const
{
  ZStackBlockGrid *grid = new ZStackBlockGrid;
  ZIntPoint newSize =
      getBlockSize() / ZIntPoint(xintv + 1, yintv + 1, zintv + 1);
  if (getBlockSize().getX() % (xintv + 1) > 0) {
    newSize.setX(newSize.getX() + 1);
  }
  if (getBlockSize().getY() % (yintv + 1) > 0) {
    newSize.setY(newSize.getY() + 1);
  }
  if (getBlockSize().getZ() % (zintv + 1) > 0) {
    newSize.setZ(newSize.getZ() + 1);
  }

  grid->setBlockSize(newSize);

  grid->setGridSize(getGridSize());
  grid->setMinPoint(getMinPoint() / ZIntPoint(xintv + 1, yintv + 1, zintv + 1));

  if (isEmpty()) {
    grid->clearStack();
  } else {
    grid->m_stackArray.resize(m_stackArray.size(), NULL);
    for (size_t i = 0; i < m_stackArray.size(); ++i) {
      ZStack *stack = m_stackArray[i];

      if (stack != NULL) {
        ZStack *dsStack = stack->clone();
        dsStack->downsampleMin(xintv, yintv, zintv);
        grid->m_stackArray[i] = dsStack;
      }
    }
  }

  return grid;
}

ZStackBlockGrid* ZStackBlockGrid::makeDownsample(const ZIntPoint &dsIntv) const
{
  return makeDownsample(dsIntv.getX(), dsIntv.getY(), dsIntv.getZ());
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

void ZStackBlockGrid::read(std::istream &stream)
{
  m_size.read(stream);
  m_minPoint.read(stream);
  m_blockSize.read(stream);

  int count = 0;
  neutube::read(stream, count);
  int maxIndex = -1;
  neutube::read(stream, maxIndex);

  if (maxIndex >= 0) {
    m_stackArray.resize(maxIndex + 1, 0);
    for (int i = 0; i < count; ++i) {
      int index = -1;
      neutube::read(stream, index);
      if (index >= 0) {
        ZStack *stack = new ZStack;
        stack->read(stream);
        m_stackArray[index] = stack;
      }
    }
  }
}

void ZStackBlockGrid::write(std::ostream &stream) const
{
  m_size.write(stream);
  m_minPoint.write(stream);
  m_blockSize.write(stream);

  int count = 0;
  int maxIndex = -1;
  for (int i = 0; i < (int) m_stackArray.size(); ++i) {
    ZStack *stack = m_stackArray[i];
    if (stack != NULL) {
      ++count;
      maxIndex = i;
    }
  }

  neutube::write(stream, count);
  neutube::write(stream, maxIndex);

  for (int i = 0; i < (int) m_stackArray.size(); ++i) {
    ZStack *stack = m_stackArray[i];
    if (stack != NULL) {
      neutube::write(stream, i);
      stack->write(stream);
    }
  }
}
