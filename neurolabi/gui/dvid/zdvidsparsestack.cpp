#include "zdvidsparsestack.h"
#include <QImage>

#include "zdvidinfo.h"
#include "zdvidreader.h"
#include "zpainter.h"
#include "zimage.h"

ZDvidSparseStack::ZDvidSparseStack()
{
  setTarget(ZStackObject::OBJECT_CANVAS);
}

ZStack* ZDvidSparseStack::getSlice(int z) const
{
  ZStack *stack = NULL;

  const ZObject3dScan *objectMask = m_sparseStack.getObjectMask();
  if (objectMask != NULL) {
    ZObject3dScan slice = objectMask->getSlice(z);
    ZIntCuboid box = slice.getBoundBox();
    stack = new ZStack(GREY, box, 1);
    stack->setZero();

    size_t stripeNumber = slice.getStripeNumber();
    for (size_t i = 0; i < stripeNumber; ++i) {
      const ZObject3dStripe &stripe = slice.getStripe(i);
      int y = stripe.getY();
      int z = stripe.getZ();
      int segmentNumber = stripe.getSegmentNumber();
      for (int j = 0; j < segmentNumber; ++j) {
        int x0 = stripe.getSegmentStart(j);
        int x1 = stripe.getSegmentEnd(j);

        for (int x = x0; x <= x1; ++x) {
          int v = getValue(x, y, z);
          stack->setIntValue(x, y, z, 0, v);
        }
      }
    }
  }

  return stack;
}

void ZDvidSparseStack::initBlockGrid()
{

  if (m_dvidReader.open(getDvidTarget())) {
    ZDvidInfo dvidInfo = m_dvidReader.readGrayScaleInfo();
    ZStackBlockGrid *grid = new ZStackBlockGrid;
    m_sparseStack.setGreyScale(grid);
    grid->setMinPoint(dvidInfo.getStartCoordinates());
    grid->setBlockSize(dvidInfo.getBlockSize());
    grid->setGridSize(dvidInfo.getGridSize());
  }
}

void ZDvidSparseStack::setDvidTarget(const ZDvidTarget &target)
{
  m_dvidTarget = target;
  initBlockGrid();
}

int ZDvidSparseStack::getValue(int x, int y, int z) const
{
  int v = 0;
  ZStackBlockGrid *stackGrid = const_cast<ZStackBlockGrid*>(
        m_sparseStack.getStackGrid());
  if (stackGrid != NULL) {
    ZStackBlockGrid::Location location = stackGrid->getLocation(x, y, z);
    const ZIntPoint& blockIndex = location.getBlockIndex();
    if (stackGrid->getStack(blockIndex) == NULL) {
      ZIntCuboid box = stackGrid->getBlockBox(blockIndex);
      ZStack *stack = m_dvidReader.readGrayScale(box);
      stackGrid->consumeStack(blockIndex, stack);
    }

    ZStack *blockStack = stackGrid->getStack(blockIndex);
    v = blockStack->getIntValueLocal(location.getLocalPosition().getX(),
                                     location.getLocalPosition().getY(),
                                     location.getLocalPosition().getZ());
  }

  return v;
}

void ZDvidSparseStack::display(
    ZPainter &painter, int slice, EDisplayStyle option) const
{
  if (slice < 0) {
    if (m_sparseStack.getObjectMask() != NULL) {
      m_sparseStack.getObjectMask()->display(painter, slice, option);
    }
  } else {
    int z = painter.getZOffset() + slice;

    ZStack *stack = getSlice(z);
    if (stack != NULL) {
      ZImage image(stack->width(), stack->height());
      image.fill(0);
      image.setData(stack, z, true, false);
      painter.drawImage(stack->getOffset().getX(), stack->getOffset().getY(),
                        image);
    }
  }
}

ZIntCuboid ZDvidSparseStack::getBoundBox() const
{
  ZIntCuboid box;
  if (m_sparseStack.getObjectMask() != NULL) {
    box = m_sparseStack.getObjectMask()->getBoundBox();
  }

  return box;
}

void ZDvidSparseStack::loadBody(int bodyId)
{
  ZObject3dScan *obj = new ZObject3dScan;
  m_dvidReader.readBody(bodyId, obj);
  m_sparseStack.setObjectMask(obj);
}


ZSTACKOBJECT_DEFINE_CLASS_NAME(ZDvidSparseStack)
