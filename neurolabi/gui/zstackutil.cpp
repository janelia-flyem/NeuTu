#include "zstackutil.h"

#include "zstackptr.h"
#include "zstack.hxx"

bool zstack::DsIntvGreaterThan::operator ()
(const ZStackPtr &stack1, const ZStackPtr &stack2) const
{
  return GetDsVolume(stack1) > GetDsVolume(stack2);
}

bool zstack::DsIntvGreaterThan::operator ()
(const ZStack &stack1, const ZStack &stack2) const
{
  return GetDsVolume(stack1) > GetDsVolume(stack2);
}

int zstack::GetDsVolume(const ZStackPtr &stack)
{
  return GetDsVolume(stack.get());
}

int zstack::GetDsVolume(const ZStack &stack)
{
  ZIntPoint ds = stack.getDsIntv() + 1;

  return ds.getX() * ds.getY() * ds.getZ();
}

int zstack::GetDsVolume(const ZStack *stack)
{
  if (stack != NULL) {
    return GetDsVolume(*stack);
  }

  return 0;
}

ZIntPoint zstack::FindClosestBg(const ZStack *stack, int x, int y, int z)
{
  if (stack->getIntValue(x, y, z) == 0) {
    return ZIntPoint(x, y, z);
  }

  size_t voxelCount = stack->getVoxelNumber();
#ifdef _DEBUG_2
  C_Stack::printValue(stack->c_stack());
#endif

  ZIntPoint rawCoord = ZIntPoint(x, y, z) - stack->getOffset();
  ssize_t index = C_Stack::indexFromCoord(
        rawCoord.getX(), rawCoord.getY(), rawCoord.getZ(),
        stack->width(), stack->height(), stack->depth());
  ZIntPoint pt;
  pt.invalidate();

  if (index >= 0) {
    long int *label = new long int[voxelCount];
    Stack *out = C_Stack::Bwdist(stack->c_stack(), NULL, label);
    C_Stack::kill(out);
#ifdef _DEBUG_2
    for (size_t i = 0; i < voxelCount; ++i) {
      std::cout << i << " " << label[i] << std::endl;
    }
#endif

    int labelIndex = label[index];
    if (stack->getIntValue(labelIndex) == 0) {
      int labelX = 0;
      int labelY = 0;
      int labelZ = 0;
      C_Stack::indexToCoord(labelIndex, stack->width(), stack->height(),
                            &labelX, &labelY, &labelZ);

      pt.set(labelX, labelY, labelZ);
      pt += stack->getOffset();
    }
    delete []label;
  }

  return pt;
}
