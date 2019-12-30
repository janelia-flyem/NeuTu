#include "zstackdocproxy.h"

#include "mvc/zstackdoc.h"
#include "zmesh.h"

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
      objList = doc->getObjectList(ZStackObject::EType::TYPE_SWC);
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

void ZStackDocProxy::SaveMesh(
    ZStackDoc *doc, const QString &fileName,
    std::function<bool (const ZMesh *)> pred)
{
  QMutexLocker locker(doc->getObjectGroup().getMutex());
  QList<ZStackObject*> meshList = doc->getObjectGroup().getObjectListUnsync(
        ZMesh::GetType());

  std::vector<ZMesh*> visibleMeshList;
  for (ZStackObject *obj : meshList) {
    ZMesh *mesh = dynamic_cast<ZMesh*>(obj);
    if (mesh) {
      if (pred(mesh)) {
        visibleMeshList.push_back(mesh);
      }
    }
  }

  ZMesh mesh = ZMesh::Merge(visibleMeshList);
  mesh.save(fileName.toStdString());
}

void ZStackDocProxy::SaveVisibleMesh(
    ZStackDoc *doc, const QString &fileName)
{
  SaveMesh(doc, fileName, [](const ZMesh *mesh) {
    return mesh->isVisible();
  });
}
