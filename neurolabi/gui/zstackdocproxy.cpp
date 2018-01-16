#include "zstackdocproxy.h"

ZStackDocProxy::ZStackDocProxy()
{

}

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

QList<ZMesh*> ZStackDocProxy::GetGeneralMeshList(ZStackDoc *doc)
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
