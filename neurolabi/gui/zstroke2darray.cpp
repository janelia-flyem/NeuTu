#include "zstroke2darray.h"
#include "zstack.hxx"
#include "zobject3d.h"

ZStroke2dArray::ZStroke2dArray()
{
}

ZStroke2dArray::~ZStroke2dArray()
{
  for (iterator iter = begin(); iter != end(); ++iter) {
    delete *iter;
  }
}

void ZStroke2dArray::labelStack(Stack *stack) const
{
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    const ZStroke2d *stroke = *iter;
    stroke->labelGrey(stack);
  }
}

void ZStroke2dArray::labelStack(ZStack *stack) const
{
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    const ZStroke2d *stroke = *iter;
    stroke->labelStack(stack);
  }
}

ZCuboid ZStroke2dArray::getBoundBox() const
{
  ZCuboid box;
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    const ZStroke2d *stroke = *iter;
    ZCuboid subbox = stroke->getBoundBox();
    box.bind(subbox);
  }

  return box;
}

ZStack* ZStroke2dArray::toStack() const
{
  ZCuboid box = getBoundBox();

  ZStack *stack = NULL;
  if (box.isValid()) {
    stack = new ZStack(GREY, box.toIntCuboid(), 1);
    for (const_iterator iter = begin(); iter != end(); ++iter) {
      const ZStroke2d *stroke = *iter;
      stroke->labelStack(stack);
    }
  }

  return stack;
}

ZObject3d* ZStroke2dArray::toObject3d() const
{
  ZStack *stack = toStack();
  ZObject3d *obj = NULL;
  if (stack != NULL) {
    obj = new ZObject3d;
    obj->loadStack(stack);
  }

  return obj;
}
