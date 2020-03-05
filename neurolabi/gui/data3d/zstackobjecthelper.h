#ifndef ZSTACKOBJECTHELPER_H
#define ZSTACKOBJECTHELPER_H

class ZStackObject;
class ZAffinePlane;

class ZStackObjectHelper
{
public:
  ZStackObjectHelper();

  static bool IsOverSize(const ZStackObject &obj);
  static void SetOverSize(ZStackObject *obj);
  static bool IsEmptyTree(const ZStackObject *obj);

  static ZStackObject* Clone(ZStackObject *obj);
  static void Align(ZStackObject *obj, const ZAffinePlane &cutPlane);
};

#endif // ZSTACKOBJECTHELPER_H
