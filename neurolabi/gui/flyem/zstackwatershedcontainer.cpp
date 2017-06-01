#include "zstackwatershedcontainer.h"

#include "imgproc/zstackwatershed.h"
#include "zstack.hxx"
#include "zobject3d.h"
#include "zstroke2d.h"
#include "zsparsestack.h"

ZStackWatershedContainer::ZStackWatershedContainer(ZStack *stack)
{
  init();
  if (m_stack != NULL) {
    m_range = m_stack->getBoundBox();
    m_source = stack;
  }
}

ZStackWatershedContainer::ZStackWatershedContainer(ZSparseStack *stack)
{
  init();
  m_spStack = stack;
  if (m_spStack != NULL) {
    m_range = m_spStack->getBoundBox();
  }
}

ZStackWatershedContainer::~ZStackWatershedContainer()
{
  clearWorkspace();
  clearSource();
}

void ZStackWatershedContainer::init()
{
  m_stack = NULL;
  m_spStack = NULL;
  m_workspace = NULL;
  m_channel = 0;
  m_floodingZero = false;
}

void ZStackWatershedContainer::clearWorkspace()
{
  if (m_workspace != NULL) {
    Kill_Stack_Watershed_Workspace(m_workspace);
    m_workspace = NULL;
  }
}

void ZStackWatershedContainer::clearSource()
{
  if (m_stack != NULL) {
    if (m_source != m_stack) {
      delete m_source;
    }
    m_source = NULL;
  }
}

ZIntPoint ZStackWatershedContainer::getSourceDsIntv() const
{
  ZIntPoint dsIntv(0, 0, 0);
  if (m_source != NULL) {
    dsIntv = m_source->getDsIntv();
  }

  return dsIntv;
}

void ZStackWatershedContainer::addSeed(const ZObject3dScan &seed)
{
  ZIntPoint dsIntv = getSourceDsIntv();

  if (dsIntv.isZero()) {
    ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), seed);
  } else {
    ZObject3dScan newSeed = seed;
    newSeed.downsampleMax(dsIntv);
    ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), newSeed);
  }
}

void ZStackWatershedContainer::addSeed(const ZStack &seed)
{
  ZIntPoint dsIntv = getSourceDsIntv();

  if (dsIntv.isZero()) {
    ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), &seed);
  } else {
    Stack *block = C_Stack::downsampleMax(
          seed.c_stack(), dsIntv.getX(), dsIntv.getY(), dsIntv.getZ(), NULL);
    ZIntPoint pt = seed.getOffset() - getSourceOffset();
    int x0 = pt.getX() / (dsIntv.getX() + 1);
    int y0 = pt.getY() / (dsIntv.getY() + 1);
    int z0 = pt.getZ() / (dsIntv.getZ() + 1);

    C_Stack::setBlockValue(
          getWorkspace()->mask, block, x0, y0, z0, 0, STACK_WATERSHED_BARRIER);
    C_Stack::kill(block);
  }
}

void ZStackWatershedContainer::addSeed(const ZStroke2d &seed)
{
  ZStack stack;
  makeMaskStack(stack);
  seed.labelStack(&stack);
}

void ZStackWatershedContainer::addSeed(const ZObject3d &seed)
{
  ZStack stack;
  makeMaskStack(stack);
  seed.labelStack(&stack);
}

void ZStackWatershedContainer::prepareSeedMask(Stack *stack, Stack *mask)
{
  if (!m_floodingZero) {
    size_t voxelNumber = C_Stack::voxelNumber(stack);

    if (C_Stack::kind(stack) == GREY) {
      uint8_t *array = C_Stack::array8(stack);
      for (size_t i = 0; i < voxelNumber; ++i) {
        if (array[i] == 0) {
          mask->array[i] = STACK_WATERSHED_BARRIER;
        }
      }
    } else {
      uint16_t *array = C_Stack::guardedArray16(stack);
      for (size_t i = 0; i < voxelNumber; ++i) {
        if (array[i] == 0) {
          mask->array[i] = STACK_WATERSHED_BARRIER;
        }
      }
    }
  }
}

Stack* ZStackWatershedContainer::getSeedMask()
{
  Stack *mask = NULL;

  Stack_Watershed_Workspace *ws = getWorkspace();
  if (ws != NULL) {
    if (ws->mask == NULL) {
      ws->mask = C_Stack::make(
            GREY, m_range.getWidth(), m_range.getHeight(), m_range.getDepth());
      C_Stack::setZero(ws->mask);
      if (getSource() != NULL) {
        prepareSeedMask(getSource(), mask);
      }
    }
    mask = ws->mask;
  }

  return mask;
}

void ZStackWatershedContainer::makeMaskStack(ZStack &stack)
{
  stack.load(getSeedMask(), false);
  stack.setOffset(getSourceOffset());
  stack.setDsIntv(getSourceDsIntv());
}

void ZStackWatershedContainer::exportMask(const std::string &filePath)
{
  ZStack stack;
  makeMaskStack(stack);
  if (stack.hasData()) {
    stack.save(filePath);
  }
}

Stack_Watershed_Workspace* ZStackWatershedContainer::getWorkspace()
{
  if (m_workspace == NULL) {
    if (getSource() != NULL) {
      if (!m_range.isEmpty()) {
        m_workspace =
            ZStackWatershed::CreateWorkspace(getSource(), m_floodingZero);
      }
    }
  }

  return m_workspace;
}

ZIntPoint ZStackWatershedContainer::getSourceOffset() const
{
  ZIntPoint offset;
  if (m_source != NULL) {
    offset = m_source->getOffset();
  }

  return offset;
}

void ZStackWatershedContainer::setRange(
    int x0, int y0, int z0, int x1, int y1, int z1)
{
  ZIntCuboid newRange(x0, y0, z0, x1, y1, z1);
  if (m_stack != NULL) {
    newRange.intersect(m_stack->getBoundBox());
  }

  if (!newRange.equals(m_range)) {
    clearWorkspace();
    m_range = newRange;
    if (m_stack != NULL) {
      ZIntCuboid fullRange = m_stack->getBoundBox();
      if (m_source != m_stack) {
        delete m_source;
        m_source = NULL;
      }
      if (m_range.equals(fullRange)) {
        m_source = m_stack;
      } else {
        ZIntPoint corner = m_range.getFirstCorner() - fullRange.getFirstCorner();

        Stack *rawSource = C_Stack::crop(
              m_stack->c_stack(m_channel),
              corner.getX(), corner.getY(), corner.getZ(),
              m_range.getWidth(), m_range.getHeight(), m_range.getDepth(), NULL);
        m_source->consume(rawSource);
        m_source->setOffset(m_range.getFirstCorner());
      }
    }
  }
}

Stack* ZStackWatershedContainer::getSource()
{
  if (m_source == NULL && m_spStack != NULL) {
    m_source = m_spStack->makeStack(m_range);
  }

  return m_source->c_stack(m_channel);
}

ZStack* ZStackWatershedContainer::run()
{
  ZStack *result = NULL;
  Stack *source = getSource();
  if (source != NULL) {
    Stack *out = C_Stack::watershed(source, getWorkspace());
    result = new ZStack;
    result->consume(out);
    result->setOffset(getSourceOffset());
  }

  return result;
}



