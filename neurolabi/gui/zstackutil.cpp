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
