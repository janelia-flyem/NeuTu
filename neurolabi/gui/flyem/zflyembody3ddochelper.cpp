#include "zflyembody3ddochelper.h"

#include "flyem/zflyembody3ddoc.h"

ZFlyEmBody3dDocHelper::ZFlyEmBody3dDocHelper()
{

}

ZMesh* ZFlyEmBody3dDocHelper::GetMeshForSplit(ZStackDoc *doc)
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
