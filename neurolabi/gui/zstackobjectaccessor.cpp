#include "zstackobjectaccessor.h"

#include <QSet>

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


int ZStackObjectAccessor::GetLabelCount(const QList<ZStackObject *> &objList)
{
  QSet<uint64_t> labelSet;
  foreach (const ZStackObject *obj, objList) {
    labelSet.insert(obj->getLabel());
  }

  return labelSet.size();
}
