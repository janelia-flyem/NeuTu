#include "zsparsestack.h"
#include "zstack.hxx"

#define MAX_STACK_VOLUME 1847483647
ZSparseStack::ZSparseStack() :
  m_objectMask(NULL), m_stackGrid(NULL), m_stack(NULL)
{
}

ZSparseStack::~ZSparseStack()
{
  deprecate(ALL_COMPONET);
  delete m_objectMask;
  delete m_stackGrid;
}

void ZSparseStack::deprecateDependent(EComponent /*component*/)
{
}

void ZSparseStack::deprecate(EComponent component)
{
  deprecateDependent(component);

  switch (component) {
  case STACK:
    delete m_stack;
    m_stack = NULL;
    break;
  case ALL_COMPONET:
    deprecate(STACK);
    break;
  }
}

bool ZSparseStack::isDeprecated(EComponent component) const
{
  switch (component) {
  case STACK:
    return m_stack == NULL;
  case ALL_COMPONET:
    break;
  }

  return false;
}

void ZSparseStack::assignStackValue(
    ZStack *stack, const ZObject3dScan &obj, const ZStackBlockGrid &stackGrid)
{
  for (size_t i = 0; i < obj.getStripeNumber(); ++i) {
    const ZObject3dStripe &stripe = obj.getStripe(i);
    int y = stripe.getY();
    int z = stripe.getZ();
    for (int j = 0; j < stripe.getSegmentNumber(); ++j) {
      int x0 = stripe.getSegmentStart(j);
      int x1 = stripe.getSegmentEnd(j);

      for (int x = x0; x <= x1; ++x) {
        int v = stackGrid.getValue(x, y, z);
        stack->setIntValue(x, y, z, 0, v);
      }
    }
  }
}

ZStack* ZSparseStack::getStack()
{
  if (m_objectMask == NULL || m_stackGrid == NULL) {
    return NULL;
  }

  if (isDeprecated(STACK)) {
    ZIntCuboid cuboid = m_objectMask->getBoundBox();
    if (!m_objectMask->isEmpty()) {
      size_t volume = cuboid.getVolume();
      if (volume > MAX_STACK_VOLUME) {
        ZObject3dScan obj = *m_objectMask;
        m_dsIntv.set(1, 1, 1);
        obj.downsampleMax(m_dsIntv.getX(), m_dsIntv.getY(), m_dsIntv.getZ());
        ZStackBlockGrid dsGrid;
        dsGrid.setBlockSize(m_stackGrid->getBlockSize() / (m_dsIntv + 1));
        dsGrid.setGridSize(m_stackGrid->getGridSize());
        dsGrid.setMinPoint(m_stackGrid->getMinPoint() / (m_dsIntv + 1));
        m_stack =  new ZStack(GREY, obj.getBoundBox(), 1);
        assignStackValue(m_stack, obj, *m_stackGrid);
      } else {
        m_stack = new ZStack(GREY, cuboid, 1);
        assignStackValue(m_stack, *m_objectMask, *m_stackGrid);
      }
    }
  }

  return m_stack;
}

size_t ZSparseStack::getObjectVolume() const
{
  if (m_objectMask == NULL) {
    return 0;
  }

  return m_objectMask->getVoxelNumber();
}

ZStack* ZSparseStack::getSlice(int z) const
{
  ZObject3dScan slice = m_objectMask->getSlice(z);
  ZIntCuboid box = slice.getBoundBox();
  ZStack *stack = new ZStack(GREY, box, 1);
  stack->setZero();
  assignStackValue(stack, slice, *m_stackGrid);

  return stack;
}
