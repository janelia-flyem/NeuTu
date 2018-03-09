#ifndef ZSTACKOBJECTACCESSOR_H
#define ZSTACKOBJECTACCESSOR_H

#include <QList>

class ZIntCuboid;
class ZStackObject;

class ZStackObjectAccessor
{
public:
  ZStackObjectAccessor();

  static ZIntCuboid GetIntBoundBox(const ZStackObject &obj);
  static int GetLabelCount(const QList<ZStackObject*> &objList);

};

#endif // ZSTACKOBJECTACCESSOR_H
