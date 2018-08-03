#include "data3d/zstackobjecthelper.h"

#include "zstackobject.h"

ZStackObjectHelper::ZStackObjectHelper()
{

}

bool ZStackObjectHelper::IsOverSize(const ZStackObject &obj)
{
  return obj.getSource() == "oversize" || obj.getObjectId() == "oversize";
}

void ZStackObjectHelper::SetOverSize(ZStackObject *obj)
{
  if (obj != NULL) {
    obj->setObjectId("oversize");
  }
}
