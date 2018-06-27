#ifndef ZOBJSMODELFACTORY_H
#define ZOBJSMODELFACTORY_H

#include "zstackobject.h"

class ZObjsModel;
class ZStackDoc;

class ZObjsModelFactory
{
public:
  ZObjsModelFactory();

  static ZObjsModel* Make(
      ZStackObject::EType type, ZStackDoc *doc, QObject *parent);
  static ZObjsModel* Make(
      ZStackObjectRole::TRole role, ZStackDoc *doc, QObject *parent);
  static ZStackObject::EType GetCanonicalType(ZStackObject::EType type);
};

#endif // ZOBJSMODELFACTORY_H
