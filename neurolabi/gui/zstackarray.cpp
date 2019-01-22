#include "zstackarray.h"
#include "zstack.hxx"
#include "geometry/zintcuboid.h"

ZStackArray::ZStackArray()
{
}

ZStackArray::ZStackArray(const std::vector<ZStackPtr> &stackArray)
{
  insert(end(), stackArray.begin(), stackArray.end());
}

ZStackArray::~ZStackArray()
{
//  for (iterator iter = begin(); iter != end(); ++iter) {
//    delete *iter;
//  }
}

void ZStackArray::paste(ZStack *stack, int valueIgnored) const
{
  if (stack != NULL) {
    for (const_iterator iter = begin(); iter != end(); ++iter) {
      const ZStack *src = iter->get();
      src->paste(stack, valueIgnored);
    }
  }
}

ZIntCuboid ZStackArray::getBoundBox() const
{
  Cuboid_I box;
  getBoundBox(&box);

  return ZIntCuboid(box);
}

void ZStackArray::getBoundBox(Cuboid_I *box) const
{
  if (box != NULL) {
    if (empty()) {
      Cuboid_I_Set_S(box, 0, 0, 0, 0, 0, 0);
    } else {

      const_iterator iter = begin();
      ZStack *stack = iter->get();
      stack->getBoundBox(box);
      ++iter;
      for (; iter != end(); ++iter) {
        Cuboid_I singleBox;
        stack = iter->get();
        stack->getBoundBox(&singleBox);
        Cuboid_I_Union(box, &singleBox, box);
      }
    }
  }
}

void ZStackArray::downsampleMax(int xIntv, int yIntv, int zIntv)
{
  if (xIntv > 0 || yIntv > 0 || zIntv > 0) {
    for (iterator iter = begin(); iter != end(); ++iter) {
      ZStack *stack = iter->get();
      stack->downsampleMax(xIntv, yIntv, zIntv);
    }
  }
}

void ZStackArray::pushDsIntv(int dx, int dy, int dz)
{
  for (ZStackPtr &stack : *this) {
    stack->pushDsIntv(dx, dy, dz);
  }
}

void ZStackArray::pushDsIntv(const ZIntPoint &dsIntv)
{
  pushDsIntv(dsIntv.getX(), dsIntv.getY() ,dsIntv.getZ());
}

void ZStackArray::append(const ZStackPtr &stack)
{
  push_back(stack);
}

void ZStackArray::append(ZStack *stack)
{
  if (stack != NULL) {
    push_back(ZStackPtr(stack));
  }
}

void ZStackArray::append(const ZStackArray &stackArray)
{
  insert(end(), stackArray.begin(), stackArray.end());
}

std::vector<ZStack*> ZStackArray::toRawArray() const
{
  std::vector<ZStack*> rawArray;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    rawArray.push_back(iter->get());
  }

  return rawArray;
}
