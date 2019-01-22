#include "zstackptr.h"
#include "zstack.hxx"

ZStackPtr::ZStackPtr()
{

}

ZStackPtr::ZStackPtr(ZStack *stack) : std::shared_ptr<ZStack>(stack)
{

}

ZStackPtr ZStackPtr::Make()
{
  return ZStackPtr(new ZStack);
}

