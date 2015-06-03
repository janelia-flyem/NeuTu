#include "zstackarray.h"
#include "zstack.hxx"

ZStackArray::ZStackArray()
{
}

ZStackArray::ZStackArray(const std::vector<ZStack *> &stackArray)
{
  insert(end(), stackArray.begin(), stackArray.end());
}

ZStackArray::~ZStackArray()
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    delete *iter;
  }
}

void ZStackArray::paste(ZStack *stack, int valueIgnored) const
{
  if (stack != NULL) {
    for (const_iterator iter = begin(); iter != end(); ++iter) {
      const ZStack *src = *iter;
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
      const ZStack *stack = *iter;
      stack->getBoundBox(box);
      ++iter;
      for (; iter != end(); ++iter) {
        Cuboid_I singleBox;
        stack = *iter;
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
      ZStack *stack = *iter;
      stack->downsampleMax(xIntv, yIntv, zIntv);
    }
  }
}
