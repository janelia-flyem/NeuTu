#include "zflyemmeshfactory.h"

#include "common/neutudefs.h"

#include "zobject3dscan.h"
#include "zmesh.h"
#include "dvid/zdvidreader.h"
#include "zmeshutils.h"

ZFlyEmMeshFactory::ZFlyEmMeshFactory()
{

}

ZMesh* ZFlyEmMeshFactory::makeMesh(uint64_t /*bodyId*/) const
{
  return NULL;
}

ZMesh* ZFlyEmMeshFactory::MakeRoiMesh(
      const ZDvidReader &reader, const std::string &roiName)
{
  ZObject3dScan obj;

  reader.readRoi(roiName, &obj);

  ZMesh *mesh = MakeMesh(obj);

  if (mesh) {
//    ZMesh decimated = ZMeshUtils::Decimate(*mesh, 0.9);
//    *mesh = ZMeshUtils::Smooth(decimated);
//    *mesh = decimated;
  }

  return mesh;
}
