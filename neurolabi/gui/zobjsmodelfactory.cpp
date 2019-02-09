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
  if (type == ZStackObject::EType::PUNCTUM) {
    type = ZStackObject::EType::PUNCTA;
  }

  return type;
}

ZObjsModel* ZObjsModelFactory::Make(
    ZStackObject::EType type, ZStackDoc *doc, QObject *parent)
{
  switch (GetCanonicalType(type)) {
  case ZStackObject::EType::SWC:
    return new ZSwcObjsModel(doc, parent);
  case ZStackObject::EType::PUNCTA:
    return new ZPunctaObjsModel(doc, parent);
  case ZStackObject::EType::GRAPH_3D:
    return new ZGraphObjsModel(doc, parent);
  case ZStackObject::EType::MESH:
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
