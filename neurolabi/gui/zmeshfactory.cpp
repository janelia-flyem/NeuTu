#include "zmeshfactory.h"

//#include <QElapsedTimer>
#include "zobject3dscan.h"
#include "zmesh.h"
#include "zstack.hxx"
#include "tz_stack_neighborhood.h"
#include "geometry/zintcuboid.h"
#include "misc/miscutility.h"
#include "tz_stack_bwmorph.h"
#include "misc/zmarchingcube.h"
#include "ilastik/laplacian_smoothing.h"
#include "zobject3dscanarray.h"
#include "data3d/zstackobjecthelper.h"

ZMeshFactory::ZMeshFactory()
{

}

void ZMeshFactory::setDsIntv(int dsIntv)
{
  m_dsIntv = dsIntv;
}

void ZMeshFactory::setSmooth(int smooth)
{
  m_smooth = smooth;
}

void ZMeshFactory::setOffsetAdjust(bool on)
{
  m_offsetAdjust = on;
}

ZMesh* ZMeshFactory::MakeMesh(const ZObject3dScan &obj)
{
  return MakeMesh(obj, 0, 3, true);
}

ZMesh* ZMeshFactory::makeMesh(const ZObject3dScan &obj)
{
  return MakeMesh(obj, m_dsIntv, m_smooth, m_offsetAdjust);
}

ZMesh* ZMeshFactory::makeMesh(const ZObject3dScanArray &objArray)
{
  ZMesh *mesh = NULL;

  std::vector<ZMesh*> meshArray;
  bool isOverSize = false;
  for (const ZObject3dScan *obj : objArray) {
    ZMesh *submesh = makeMesh(*obj);
//    mesh->prepareNormals();
    if (submesh != NULL) {
      meshArray.push_back(submesh);
      if (ZStackObjectHelper::IsOverSize(*submesh)) {
        isOverSize = true;
      }
    }
  }

  if (!meshArray.empty()) {
    mesh = meshArray[0];
//    QElapsedTimer timer;
//    timer.start();
    for (size_t i = 1; i < meshArray.size(); ++i) {
      ZMesh *currentMesh = meshArray[i];
      if (mesh->numTriangles() < currentMesh->numTriangles()) {
        std::swap(mesh, currentMesh);
      }
      mesh->append(*currentMesh);
      delete currentMesh;
    }
//    LINFO() << "Mesh appending time:" << timer.elapsed() << "ms";

    if (isOverSize) {
      ZStackObjectHelper::SetOverSize(mesh);
    }

//    mesh->prepareNormals();
  }

  return mesh;
}
/*
ZMesh* ZMeshFactory::MakeMesh(const ZObject3dScanArray &objArray)
{
  ZMesh *mesh = NULL;

  std::vector<ZMesh*> meshArray;
  for (const ZObject3dScan *obj : objArray) {
    ZMesh *mesh = MakeMesh(*obj);
//    mesh->prepareNormals();
    meshArray.push_back(mesh);
  }

  if (!meshArray.empty()) {
    mesh = meshArray[0];
    for (size_t i = 1; i < meshArray.size(); ++i) {
      ZMesh *currentMesh = meshArray[i];
      if (mesh->numTriangles() < currentMesh->numTriangles()) {
        std::swap(mesh, currentMesh);
      }
      mesh->append(*currentMesh);
      delete currentMesh;
    }
  }

  return mesh;
}
*/
#if 0
ZMesh* ZMeshFactory::MakeMesh(
    const ZObject3dScan &obj, const ZIntPoint &dsIntv, int smooth)
{
  if (obj.isEmpty()) {
    return NULL;
  }

  ZObject3dScan dsObj = obj;

  if (!dsIntv.isValid()) {
    ZIntCuboid box = dsObj.getBoundBox();
    dsIntv = misc::getIsoDsIntvFor3DVolume(box, neutube::ONEGIGA / 2, true);
  }

  if (dsIntv.semiDefinitePositive()) {
    dsObj.downsampleMax(dsIntv, dsIntv, dsIntv);
  }

  ZStack *stack = dsObj.toStackObjectWithMargin(1, 1);
  ZMesh *mesh = ZMarchingCube::March(*stack, smooth, NULL);

  if (dsIntv.semiDefinitePositive() && mesh != NULL) {
    mesh->setObjectId("oversize");
  }

  delete stack;

  return mesh;
}
#endif

ZMesh* ZMeshFactory::MakeMesh(
    const ZObject3dScan &obj, int dsIntv, int smooth, bool offsetAdjust)
{
  if (!obj.hasVoxel()) {
    return NULL;
  }

  ZObject3dScan dsObj = obj;

  if (dsIntv == 0) {
    ZIntCuboid box = dsObj.getBoundBox();
    dsIntv = misc::getIsoDsIntvFor3DVolume(box, neutu::ONEGIGA / 2, true);
  }

  if (dsIntv > 0) {
    dsObj.downsampleMax(dsIntv, dsIntv, dsIntv);
  }

  ZStack *stack = dsObj.toStackObjectWithMargin(1, 1);
  ZMesh *mesh = ZMarchingCube::March(*stack, smooth, offsetAdjust, NULL);

  if (dsIntv > 0 && mesh != NULL) {
    ZStackObjectHelper::SetOverSize(mesh);
  }

  delete stack;

  return mesh;
}


ZMesh* ZMeshFactory::MakeFaceMesh(const ZObject3dScan &obj, int dsIntv)
{
  if (obj.isEmpty()) {
    return NULL;
  }

  ZMesh *mesh = new ZMesh;

  ZObject3dScan dsObj = obj;

  if (dsIntv == 0) {
    ZIntCuboid box = dsObj.getBoundBox();
    dsIntv = misc::getIsoDsIntvFor3DVolume(box, neutu::ONEGIGA / 2, true);
  }

  if (dsIntv > 0) {
    dsObj.downsampleMax(dsIntv, dsIntv, dsIntv);
  }

  //For each voxel, create a graph
  int startCoord[3];
  Stack *stack = dsObj.toStackWithMargin(startCoord, 1, 1);

#if 0
  Stack *out = Stack_Fillhole(stack, NULL, 1);
  C_Stack::kill(stack);
  stack = out;
#endif

  size_t offset = 0;
  int i, j, k;
  int neighbor[26];
  int width = C_Stack::width(stack);
  int height = C_Stack::height(stack);
  int depth = C_Stack::depth(stack);
  int cwidth = width - 1;
  int cheight = height - 1;
  int cdepth = depth - 1;

  Stack_Neighbor_Offset(6, C_Stack::width(stack), C_Stack::height(stack), neighbor);
  uint8_t *array = C_Stack::array8(stack);

  std::vector<glm::vec3> coordLlfs;
  std::vector<glm::vec3> coordUrbs;
  std::vector<std::vector<bool> > faceVisbility;
//  std::vector<glm::vec4> cubeColors;

//  qreal r,g,b,a;
//  color.getRgbF(&r, &g, &b, &a); // QColor -> glm::vec4

  int bw = dsObj.getDsIntv().getX() + 1;
  int bh = dsObj.getDsIntv().getY() + 1;
  int bd = dsObj.getDsIntv().getZ() + 1;

  int lastX = 0;

  for (k = 0; k <= cdepth; k ++) {
    for (j = 0; j <= cheight; j++) {
      bool hasLast = false;
      for (i = 0; i <= cwidth; i++) {
        if (array[offset] > 0) {
          std::vector<bool> fv(6, false);
          bool visible = false;
          for (int n = 0; n < 6; n++) {
            if (array[offset + neighbor[n]] == 0) {
              fv[n] = true;
              visible = true;
            }
          }
          if (visible) {
            ZIntCuboid box;
            box.setFirstCorner(
                  (i + startCoord[0]) * bw, (j + startCoord[1]) * bh,
                (k + startCoord[2]) * bd);
            box.setLastCorner(box.getFirstCorner() + ZIntPoint(bw, bh, bd));

            bool added = false;
            if (hasLast) {
              if (box.getFirstCorner().getX() == lastX) {
                std::vector<bool> &lastFv = faceVisbility.back();

                if (fv[2] == lastFv[2] && fv[3] == lastFv[3] &&
                    fv[4] == lastFv[4] && fv[5] == lastFv[5]) {
                  glm::vec3 &coord = coordUrbs.back();
                  coord[0] = box.getLastCorner().getX();
                  lastFv[1] = fv[1];
                  added = true;
                }
              }
            }

            if (!added) {
              coordLlfs.emplace_back(box.getFirstCorner().getX(),
                                     box.getFirstCorner().getY(),
                                     box.getFirstCorner().getZ());
              coordUrbs.emplace_back(box.getLastCorner().getX(),
                                     box.getLastCorner().getY(),
                                     box.getLastCorner().getZ());
              //              cubeColors.emplace_back(r, g, b, a);
              faceVisbility.push_back(fv);
              hasLast = true;
            }
            lastX = box.getLastCorner().getX();
          }
        }
        offset++;
      }
    }
  }

  C_Stack::kill(stack);

  mesh->createCubesWithoutNormal(coordLlfs, coordUrbs, faceVisbility, NULL);

  return mesh;
}
