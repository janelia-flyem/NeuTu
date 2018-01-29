#include "zstackobjectaccessor.h"

#include "zstackobject.h"
#include "zintcuboid.h"

ZStackObjectAccessor::ZStackObjectAccessor()
{

}

ZIntCuboid ZStackObjectAccessor::GetIntBoundBox(const ZStackObject &obj)
{
  ZIntCuboid box;
  obj.boundBox(&box);

  return box;
}
