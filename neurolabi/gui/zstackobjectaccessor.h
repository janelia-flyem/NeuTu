#ifndef ZSTACKOBJECTACCESSOR_H
#define ZSTACKOBJECTACCESSOR_H

class ZIntCuboid;
class ZStackObject;

class ZStackObjectAccessor
{
public:
  ZStackObjectAccessor();

  static ZIntCuboid GetIntBoundBox(const ZStackObject &obj);

};

#endif // ZSTACKOBJECTACCESSOR_H
