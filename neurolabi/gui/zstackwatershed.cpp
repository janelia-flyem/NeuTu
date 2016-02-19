#include "zstackwatershed.h"
#include "neutubeconfig.h"
#include "tz_cuboid_i.h"
#include "tz_math.h"
#include "zstack.hxx"


ZStackWatershed::ZStackWatershed() : m_floodingZero(false)
{
  Cuboid_I_Set_S(&m_range, 0, 0, 0, 0, 0, 0);
}

ZStackWatershed::~ZStackWatershed()
{

}

Stack_Watershed_Workspace* ZStackWatershed::createWorkspace(const Stack *stack)
{
  Stack_Watershed_Workspace *ws = Make_Stack_Watershed_Workspace(stack);
  ws->conn =26;
  if (C_Stack::kind(stack) == GREY) {
    ws->start_level = 255;
  } else {
    ws->start_level = 65535;
  }
  std::cout << "workspace mask size: " << C_Stack::width(stack) << "x"
            << C_Stack::height(stack) << "x" << C_Stack::depth(stack)
            << std::endl;
  Stack *mask = C_Stack::make(GREY, C_Stack::width(stack),
                              C_Stack::height(stack), C_Stack::depth(stack));
  ws->mask = mask;
  C_Stack::setZero(mask);
  size_t voxelNumber = C_Stack::voxelNumber(stack);
  for (size_t i = 0; i < voxelNumber; ++i) {
    //Need modifcation to handle 16bit data
    if (stack->array[i] == 0) {
      mask->array[i] = STACK_WATERSHED_BARRIER;
    }
  }

  return ws;
}

void ZStackWatershed::addSeed(Stack_Watershed_Workspace *ws,
                              const ZIntPoint &offset,
                              const std::vector<ZStack*> &seedMask)
{
  if (ws != NULL) {
    for (std::vector<ZStack*>::const_iterator iter = seedMask.begin();
         iter != seedMask.end(); ++iter) {
      ZStack *seed = *iter;
      Stack *block = seed->c_stack();
      int x0 = seed->getOffset().getX() - offset.getX();
      int y0 = seed->getOffset().getY() - offset.getY();
      int z0 = seed->getOffset().getZ() - offset.getZ();

      C_Stack::setBlockValue(
            ws->mask, block, x0, y0, z0, 0, STACK_WATERSHED_BARRIER);
    }
  }
}

void ZStackWatershed::setRange(int x0, int y0, int z0, int x1, int y1, int z1)
{
  m_range.cb[0] = x0;
  m_range.cb[1] = y0;
  m_range.cb[2] = z0;
  m_range.ce[0] = x1;
  m_range.ce[1] = y1;
  m_range.ce[2] = z1;
}

void ZStackWatershed::setRange(const Cuboid_I &box)
{
  m_range = box;
}

ZStack* ZStackWatershed::run(const ZStack *stack, const ZStack* seedMask)
{
  std::vector<ZStack *> seedArray;
  seedArray.push_back(const_cast<ZStack*>(seedMask));

  return run(stack, seedArray);
}

ZStack *ZStackWatershed::run(
    const ZStack *stack, const std::vector<ZStack *> &seedMask)
{
  ZStack *result = NULL;
  const Stack *source = stack->c_stack();

  if (stack != NULL) {
    Cuboid_I stackBox;
    stack->getBoundBox(&stackBox);

    Cuboid_I box = m_range;
    for (int i = 0; i < 3; ++i) {
      if (m_range.ce[i] < m_range.cb[i]) {
        box.ce[i] = stackBox.ce[i];
      }
    }

    std::cout << "Estimating bounding box ..." << std::endl;
    Cuboid_I_Intersect(&stackBox, &box, &box);

    if (Cuboid_I_Is_Valid(&box)) {
      ZIntPoint sourceOffset = ZIntPoint(box.cb[0], box.cb[1], box.cb[2]);
      Cuboid_I_Translate(
            &box, -stackBox.cb[0], -stackBox.cb[1], -stackBox.cb[2]);
      source = C_Stack::crop(stack->c_stack(), box, NULL);

      Stack_Watershed_Workspace *ws = createWorkspace(source);

      addSeed(ws, sourceOffset, seedMask);

#ifdef _DEBUG_2
      C_Stack::write(GET_DATA_DIR + "/test_seed.tif", ws->mask);
#endif

      std::cout << "Computing watershed ..." << std::endl;

#ifdef _DEBUG_2
      Stack *out2 = C_Stack::make(GREY, stack->width(), stack->height(), stack->depth());

      Zero_Stack(out2);
      Stack *out = C_Stack::watershed(source, ws, out2);
#else
      Stack *out = Stack_Watershed(source, ws);
#endif
      std::cout << "Creating result ..." << std::endl;
      result = new ZStack;
      result->consume(out);
      result->setOffset(sourceOffset);

      std::cout << "Cleaning space ..." << std::endl;
      if (source != stack->c_stack()) {
        C_Stack::kill(const_cast<Stack*>(source));
      }
      Kill_Stack_Watershed_Workspace(ws);

      std::cout << "Splitting done." << std::endl;
    }
  }

  return result;
}

#if 0
ZStack* ZStackWatershed::run(const Stack *stack,
                             const std::vector<ZStack *> &seedMask)
{
  ZStack *result = NULL;
  const Stack *source = stack;

  if (stack != NULL) {
    Cuboid_I box = m_range;
    if (m_range.ce[0] < 0 || m_range.ce[0] >= C_Stack::width(stack)) {
      //extend the size to the last corner
      box.ce[0] = C_Stack::width(stack) - 1;
    }
    if (m_range.ce[1] < 0 || m_range.ce[1] >= C_Stack::height(stack)) {
      //extend the size to the last corner
      box.ce[1] = C_Stack::height(stack) - 1;
    }
    if (m_range.ce[2] < 0 || m_range.ce[2] >= C_Stack::depth(stack)) {
      //extend the size to the last corner
      box.ce[2] = C_Stack::depth(stack) - 1;
    }

    Stack_Watershed_Workspace *ws = createWorkspace(source);
    addSeed(ws, ZPoint(0, 0, 0), seedMask);

#ifdef _DEBUG_2
    C_Stack::write(GET_DATA_DIR + "/test.tif", ws->mask);
#endif

    Stack *out = Stack_Watershed(source, ws);
    result = new ZStack;
    result->consumeData(out);
    result->setOffset(box.cb[0], box.cb[1], box.cb[2]);

    if (source != stack) {
      C_Stack::kill(const_cast<Stack*>(source));
    }
    Kill_Stack_Watershed_Workspace(ws);
  }

  return result;
}
#endif
