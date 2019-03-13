#ifndef ZFLYEMBODY3DDOCHELPER_H
#define ZFLYEMBODY3DDOCHELPER_H

class ZStackDoc;
class ZMesh;

class ZFlyEmBody3dDocHelper
{
public:
  ZFlyEmBody3dDocHelper();

  static ZMesh* GetMeshForSplit(ZStackDoc *doc);
};

#endif // ZFLYEMBODY3DDOCHELPER_H
