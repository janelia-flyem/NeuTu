#include "zstackptr.h"
#include "zstack.hxx"

ZStackPtr::ZStackPtr()
{

}

ZStackPtr::ZStackPtr(ZStack *stack) : ZSharedPointer<ZStack>(stack)
{

}

ZStackPtr ZStackPtr::Make()
{
  return ZStackPtr(new ZStack);
}
