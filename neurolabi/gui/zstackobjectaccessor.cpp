#include "zstackobjectaccessor.h"

#include <QSet>

#include "zstackobject.h"
#include "geometry/zintcuboid.h"
#include "zswctree.h"

ZStackObjectAccessor::ZStackObjectAccessor()
{

}

ZIntCuboid ZStackObjectAccessor::GetIntBoundBox(const ZStackObject &obj)
{
  ZIntCuboid box;
  obj.boundBox(&box);

  return box;
}


int ZStackObjectAccessor::GetLabelCount(const QList<ZStackObject*> &objList)
{
  return GetLabelSet(objList).size();
}

QSet<uint64_t> ZStackObjectAccessor::GetLabelSet(const QList<ZSwcTree*> &objList)
{
  return GetLabelSet<ZSwcTree>(objList);
}

/*
QSet<uint64_t> ZStackObjectAccessor::GetLabelSet(
    const QList<ZStackObject*> &objList)
{
  QSet<uint64_t> labelSet;
  foreach (const ZStackObject *obj, objList) {
    labelSet.insert(obj->getLabel());
  }

  return labelSet;
}
*/
