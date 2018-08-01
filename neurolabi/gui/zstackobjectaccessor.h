#ifndef ZSTACKOBJECTACCESSOR_H
#define ZSTACKOBJECTACCESSOR_H

#include <QList>
#include <QSet>

class ZIntCuboid;
class ZStackObject;
class ZSwcTree;

class ZStackObjectAccessor
{
public:
  ZStackObjectAccessor();

  static ZIntCuboid GetIntBoundBox(const ZStackObject &obj);
  static int GetLabelCount(const QList<ZStackObject*> &objList);
//  static QSet<uint64_t> GetLabelSet(const QList<ZStackObject*> &objList);

  template<typename T>
  static QSet<uint64_t> GetLabelSet(const QList<T*> &objList);

  static QSet<uint64_t> GetLabelSet(const QList<ZSwcTree*> &objList);
};

template<typename T>
QSet<uint64_t> ZStackObjectAccessor::GetLabelSet(const QList<T*> &objList)
{
  QSet<uint64_t> labelSet;
  foreach (const T *obj, objList) {
    labelSet.insert(obj->getLabel());
  }

  return labelSet;
}


#endif // ZSTACKOBJECTACCESSOR_H
