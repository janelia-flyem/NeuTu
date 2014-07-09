#include "zstackfactory.h"

ZStackFactory::ZStackFactory()
{
}

Stack *ZStackFactory::pileMatched(const std::vector<Stack*> stackArray)
{
  if (stackArray.empty()) {
    return NULL;
  }

  int kind = C_Stack::kind(stackArray[0]);
  int width = C_Stack::width(stackArray[0]);
  int height = C_Stack::height(stackArray[0]);
  int depth = 0;
  for (std::vector<Stack*>::const_iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    depth += C_Stack::depth(*iter);
  }

  Stack *out = C_Stack::make(kind, width, height, depth);

  int currentPlane = 0;
  for (std::vector<Stack*>::const_iterator iter = stackArray.begin();
       iter != stackArray.end(); ++iter) {
    const Stack* stack = *iter;
    Stack substack = C_Stack::sliceView(
          out, currentPlane, currentPlane + C_Stack::depth(stack) - 1);
    C_Stack::copyValue(stack, &substack);
    currentPlane += C_Stack::depth(stack);
  }

  return out;
}

ZStack* ZStackFactory::makeStack(ZStack *stack) const
{
  return stack;
}

ZStack* ZStackFactory::makeVirtualStack(int width, int height, int depth)
{
  if (width <= 0 || height <= 0 || depth <= 0) {
    return NULL;
  }

  return new ZStack(GREY, width, height, depth, 1, true);
}

ZStack* ZStackFactory::makeVirtualStack(const ZIntCuboid &box)
{
  ZStack *stack =
      makeVirtualStack(box.getWidth(), box.getHeight(), box.getDepth());
  if (stack != NULL) {
    stack->setOffset(box.getFirstCorner());
  }

  return stack;
}

ZStack* ZStackFactory::makeOneStack(int width, int height, int depth)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, 1);
  stack->setOne();

  return stack;
}

ZStack* ZStackFactory::makeIndexStack(int width, int height, int depth)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, 1);
  size_t voxelNumber = stack->getVoxelNumber();
  uint8_t *array = stack->array8();
  for (size_t i = 0; i < voxelNumber; ++i) {
    array[i] = i;
  }

  return stack;
}

ZStack* ZStackFactory::makeUniformStack(int width, int height, int depth, int v)
{
  ZStack *stack = new ZStack(GREY, width, height, depth, 1);
  size_t voxelNumber = stack->getVoxelNumber();
  uint8_t *array = stack->array8();
  for (size_t i = 0; i < voxelNumber; ++i) {
    array[i] = v;
  }

  return stack;
}
