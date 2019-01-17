#ifndef ZSTACKOBJECTHELPER_H
#define ZSTACKOBJECTHELPER_H

class ZStackObject;

class ZStackObjectHelper
{
public:
  ZStackObjectHelper();

  static bool IsOverSize(const ZStackObject &obj);
  static void SetOverSize(ZStackObject *obj);
  static bool IsEmptyTree(const ZStackObject *obj);
};

#endif // ZSTACKOBJECTHELPER_H
