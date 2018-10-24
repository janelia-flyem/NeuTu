#include "zstackdocproxy.h"

#include "flyem/zflyembody3ddoc.h"

ZStackDocProxy::ZStackDocProxy()
{

}

#if 0
QList<ZStackObject*> ZStackDocProxy::GetObjectList(
    ZStackDoc *doc, ZStackDoc::EDocumentDataType dataType)
{
  QList<ZStackObject*> objList;

  if (doc != NULL) {
    switch (dataType) {
    case ZStackDoc::DATA_SWC:
      objList = doc->getObjectList(ZStackObject::TYPE_SWC);
      break;
    case ZStackDoc::DATA_TODO:
      objList = doc->getObjectList(ZStackObject::TYPE_FLYEM_TODO_LIST);
      break;
    default:
      break;
    }
  }

  return objList;
}
#endif

QList<ZMesh*> ZStackDocProxy::GetGeneralMeshList(const ZStackDoc *doc)
{
  return GetNonRoiMeshList(doc);
}

QList<ZMesh*> ZStackDocProxy::GetNonRoiMeshList(const ZStackDoc *doc)
{
  QList<ZMesh*> filteredMeshList;

  if (doc != NULL) {
    QList<ZMesh*> meshList = doc->getMeshList();

    foreach(ZMesh *mesh, meshList) {
      if (!mesh->hasRole(ZStackObjectRole::ROLE_ROI)) {
        filteredMeshList.append(mesh);
      }
    }
  }

  return filteredMeshList;
}

QList<ZMesh*> ZStackDocProxy::GetBodyMeshList(const ZStackDoc *doc)
{
  QList<ZMesh*> filteredMeshList;

  if (doc != NULL) {
    QList<ZMesh*> meshList = doc->getMeshList();

    foreach(ZMesh *mesh, meshList) {
      if (!mesh->hasRole(ZStackObjectRole::ROLE_ROI) &&
          !mesh->hasRole(ZStackObjectRole::ROLE_SUPERVOXEL)) { //todo: use body role
        filteredMeshList.append(mesh);
      }
    }
  }

  return filteredMeshList;
}

QList<ZMesh*> ZStackDocProxy::GetRoiMeshList(ZStackDoc *doc)
{
  QList<ZMesh*> filteredMeshList;

  if (doc != NULL) {
    QList<ZMesh*> meshList = doc->getMeshList();

    foreach(ZMesh *mesh, meshList) {
      if (mesh->hasRole(ZStackObjectRole::ROLE_ROI)) {
        filteredMeshList.append(mesh);
      }
    }
  }

  return filteredMeshList;
}

ZMesh* ZStackDocProxy::GetMeshForSplit(ZStackDoc *doc)
{
  ZMesh *mesh = NULL;

  ZFlyEmBody3dDoc *bodyDoc = qobject_cast<ZFlyEmBody3dDoc*>(doc);
  if (bodyDoc != NULL) {
    mesh = bodyDoc->getMeshForSplit();
  } else {
    QList<ZMesh*> meshList = doc->getMeshList();
    if (!meshList.isEmpty()) {
      mesh = meshList.front();
    }
  }

  return mesh;
}
