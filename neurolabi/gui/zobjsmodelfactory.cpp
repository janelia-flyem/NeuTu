#include "zobjsmodelfactory.h"

#include "zswcobjsmodel.h"
//#include "zswcnodeobjsmodel.h"
#include "zpunctaobjsmodel.h"
#include "zdocplayerobjsmodel.h"
#include "zgraphobjsmodel.h"
//#include "zsurfaceobjsmodel.h"
#include "zmeshobjsmodel.h"
#include "zroiobjsmodel.h"

ZObjsModelFactory::ZObjsModelFactory()
{

}

ZStackObject::EType ZObjsModelFactory::GetCanonicalType(ZStackObject::EType type)
{
  if (type == ZStackObject::TYPE_PUNCTUM) {
    type = ZStackObject::TYPE_PUNCTA;
  }

  return type;
}

ZObjsModel* ZObjsModelFactory::Make(
    ZStackObject::EType type, ZStackDoc *doc, QObject *parent)
{
  switch (GetCanonicalType(type)) {
  case ZStackObject::TYPE_SWC:
    return new ZSwcObjsModel(doc, parent);
  case ZStackObject::TYPE_PUNCTA:
    return new ZPunctaObjsModel(doc, parent);
  case ZStackObject::TYPE_3D_GRAPH:
    return new ZGraphObjsModel(doc, parent);
  case ZStackObject::TYPE_MESH:
    return new ZMeshObjsModel(doc, parent);
  default:
    return NULL;
  }

  return NULL;
}

ZObjsModel* ZObjsModelFactory::Make(
    ZStackObjectRole::TRole role, ZStackDoc *doc, QObject *parent)
{
  return new ZDocPlayerObjsModel(doc, role, parent);
}
