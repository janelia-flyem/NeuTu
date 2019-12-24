#include "zstackwatershed.h"

#include <iostream>

#include "tz_cuboid_i.h"

#include "neutubeconfig.h"

#include "zstack.hxx"
#include "geometry/zintcuboid.h"
#include "zobject3dscan.h"
#include "zobject3d.h"
#include "zstackarray.h"

ZStackWatershed::ZStackWatershed() : m_floodingZero(false)
{
  Cuboid_I_Set_S(&m_range, 0, 0, 0, 0, 0, 0);
}

ZStackWatershed::~ZStackWatershed()
{

}

Stack_Watershed_Workspace* ZStackWatershed::CreateWorkspace(
    const ZIntCuboid &box, int kind)
{
  Stack_Watershed_Workspace *ws = NULL;
  if (!box.isEmpty()) {
    size_t voxelNumber = box.getVolume();

    ws = C_Stack::MakeStackWatershedWorkspace(voxelNumber);
    ws->conn =26;
    if (kind == GREY) {
      ws->start_level = 255;
    } else {
      ws->start_level = 65535;
    }
    std::cout << "workspace mask size: " << box.getWidth() << "x"
              << box.getHeight() << "x" << box.getDepth()
              << std::endl;
    Stack *mask = C_Stack::make(
          GREY, box.getWidth(), box.getHeight(), box.getDepth());
    C_Stack::setZero(mask);
    ws->mask = mask;
  }

  return ws;
}


Stack_Watershed_Workspace* ZStackWatershed::CreateWorkspace(
    const Stack *stack, bool floodingZero)
{
  if (stack == NULL) {
    return NULL;
  }

  Stack_Watershed_Workspace *ws = C_Stack::MakeStackWatershedWorkspace(stack);
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

  if (!floodingZero) {
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

  return ws;
}

void ZStackWatershed::AddSeed(
    Stack_Watershed_Workspace *ws, const ZIntPoint &offset, const ZStack *seed)
{
  if (seed != NULL) {
    const Stack *block = seed->c_stack();
    int x0 = seed->getOffset().getX() - offset.getX();
    int y0 = seed->getOffset().getY() - offset.getY();
    int z0 = seed->getOffset().getZ() - offset.getZ();

    C_Stack::setBlockValue(
          ws->mask, block, x0, y0, z0, 0, STACK_WATERSHED_BARRIER);
  }
}

void ZStackWatershed::AddSeed(Stack_Watershed_Workspace *ws,
                              const ZIntPoint &offset,
                              const std::vector<ZStack*> &seedMask)
{
  if (ws != NULL) {
    for (std::vector<ZStack*>::const_iterator iter = seedMask.begin();
         iter != seedMask.end(); ++iter) {
      ZStack *seed = *iter;
      AddSeed(ws, offset, seed);
    }
  }
}

void ZStackWatershed::AddSeed(
    Stack_Watershed_Workspace *ws, const ZIntPoint &offset,
    const ZObject3dScan &seed)
{
  if (ws != NULL) {
    uint8_t label = seed.getLabel();
    ZObject3dScan::ConstVoxelIterator voxelIter(&seed);
    while (voxelIter.hasNext()) {
      ZIntPoint pt = voxelIter.next();
      pt -= offset;

      uint8_t *array = C_Stack::array8(ws->mask, pt.getX(), pt.getY(), pt.getZ());
      if (array != NULL) {
        if (array[0] != STACK_WATERSHED_BARRIER) {
          array[0] = label;
        }
      }
    }
  }
}

void ZStackWatershed::AddSeed(
    Stack_Watershed_Workspace *ws, const ZIntPoint &offset,
    const ZIntPoint &dsIntv, const ZObject3d *seed)
{
  if (ws != NULL) {
    uint8_t label = seed->getLabel();
    int sx = dsIntv.getX() + 1;
    int sy = dsIntv.getY() + 1;
    int sz = dsIntv.getZ() + 1;
    uint8_t *array = C_Stack::array8(ws->mask);
    size_t area = C_Stack::area(ws->mask);
    int width = C_Stack::width(ws->mask);
    int height = C_Stack::height(ws->mask);
    int depth = C_Stack::depth(ws->mask);

    for (size_t i = 0; i < seed->size(); ++i) {
      int x = seed->getX(i);
      int y = seed->getY(i);
      int z = seed->getZ(i);
      x /= sx;
      y /= sy;
      z /= sz;
      x -= offset.getX();
      y -= offset.getY();
      z -= offset.getZ();
      if (x >= 0 && x < width && y >= 0 && y < height && z >= 0 && z < depth) {
        uint8_t *p = array + z * area + y * width + x;
        if (*p != STACK_WATERSHED_BARRIER) {
          if (*p > 0) {
            if (*p != label ) { //Seed conflict
              *p = 0;
            }
          } else {
            *p = label;
          }
        }
      }
    }
  }
}

void ZStackWatershed::AddSeed(
    Stack_Watershed_Workspace *ws, const ZIntPoint &offset, const ZIntPoint &dsIntv,
    const std::vector<ZObject3d *> &seedArray)
{
  for (std::vector<ZObject3d*>::const_iterator iter = seedArray.begin();
       iter != seedArray.end(); ++iter) {
    const ZObject3d *obj = *iter;
    AddSeed(ws, offset, dsIntv, obj);
  }
}

void ZStackWatershed::AddSeed(
    Stack_Watershed_Workspace *ws, const ZIntPoint &offset,
    const std::vector<ZObject3dScan *> &seedArray)
{
  for (std::vector<ZObject3dScan*>::const_iterator iter = seedArray.begin();
       iter != seedArray.end(); ++iter) {
    const ZObject3dScan *obj = *iter;
    AddSeed(ws, offset, *obj);
  }
}

void ZStackWatershed::AddSeed(
    Stack_Watershed_Workspace *ws, const ZIntPoint &offset,
    const std::vector<ZObject3dScan> &seedArray)
{
  for (std::vector<ZObject3dScan>::const_iterator iter = seedArray.begin();
       iter != seedArray.end(); ++iter) {
    AddSeed(ws, offset, *iter);
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
    const ZStack *stack, const ZStackArray &seedMask)
{
  return run(stack, seedMask.toRawArray());
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

      Stack_Watershed_Workspace *ws = CreateWorkspace(source, m_floodingZero);
      ws->conn=6;
      AddSeed(ws, sourceOffset, seedMask);

#ifdef _DEBUG_2
      C_Stack::write(GET_DATA_DIR + "/test_seed.tif", ws->mask);
#endif

      std::cout << "Computing watershed ..." << std::endl;

#ifdef _DEBUG_2
      Stack *out2 = C_Stack::make(GREY, stack->width(), stack->height(), stack->depth());

      Zero_Stack(out2);
      Stack *out = C_Stack::watershed(source, ws, out2);
#else
//      Stack *out = Stack_Watershed(source, ws);
      Stack *out = C_Stack::watershed(source, ws);
#endif
      std::cout << "Creating result ..." << std::endl;
      result = new ZStack;
      result->consume(out);
      result->setOffset(sourceOffset);

      std::cout << "Cleaning space ..." << std::endl;
      if (source != stack->c_stack()) {
        C_Stack::kill(const_cast<Stack*>(source));
      }
      C_Stack::KillStackWatershedWorkspace(ws);
//      Kill_Stack_Watershed_Workspace(ws);

      std::cout << "Splitting done." << std::endl;
    }
  }

  return result;
}
