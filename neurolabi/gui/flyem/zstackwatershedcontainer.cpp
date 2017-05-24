#include "zstackwatershedcontainer.h"

#include "imgproc/zstackwatershed.h"
#include "zstack.hxx"
#include "zobject3d.h"
#include "zstroke2d.h"

ZStackWatershedContainer::ZStackWatershedContainer(ZStack *stack)
{
  m_stack = stack;
  m_workspace = NULL;
  m_channel = 0;
  m_floodingZero = false;
  if (m_stack != NULL) {
    m_range = m_stack->getBoundBox();
    m_source = stack->c_stack(m_channel);
  }
}

ZStackWatershedContainer::~ZStackWatershedContainer()
{
  clearWorkspace();
  clearSource();
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
    if (m_source != m_stack->c_stack(m_channel)) {
      C_Stack::kill(m_source);
    }
    m_source = NULL;
  }
}

void ZStackWatershedContainer::addSeed(const ZObject3dScan &seed)
{
  ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), seed);
}

void ZStackWatershedContainer::addSeed(const ZStack &seed)
{
  ZStackWatershed::AddSeed(getWorkspace(), getSourceOffset(), &seed);
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
    if (m_source != NULL) {
      if (!m_range.isEmpty()) {
        m_workspace =
            ZStackWatershed::CreateWorkspace(m_source, m_floodingZero);
        m_sourceOffset = m_range.getFirstCorner();
      }
    }
  }

  return m_workspace;
}

ZIntPoint ZStackWatershedContainer::getSourceOffset() const
{
  return m_sourceOffset;
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
      if (m_source != m_stack->c_stack(m_channel)) {
        C_Stack::kill(m_source);
      }
      if (m_range.equals(fullRange)) {
        m_source = m_stack->c_stack(m_channel);
      } else {
        ZIntPoint corner = m_range.getFirstCorner() - fullRange.getFirstCorner();

        m_source = C_Stack::crop(
              m_stack->c_stack(m_channel),
              corner.getX(), corner.getY(), corner.getZ(),
              m_range.getWidth(), m_range.getHeight(), m_range.getDepth(), NULL);
      }
    }
  }
}

Stack* ZStackWatershedContainer::getSource()
{
  return m_source;
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



