#include "zmeshfactory.h"
#include "zobject3dscan.h"
#include "zmesh.h"
#include "tz_stack_neighborhood.h"
#include "zintcuboid.h"
#include "misc/miscutility.h"
#include "tz_stack_bwmorph.h"

ZMeshFactory::ZMeshFactory()
{

}

ZMesh* ZMeshFactory::MakeMesh(const ZObject3dScan &obj, int dsIntv)
{
  if (obj.isEmpty()) {
    return NULL;
  }

  ZObject3dScan dsObj = obj;

  if (dsIntv == 0) {
    ZIntCuboid box = dsObj.getBoundBox();
    dsIntv = misc::getIsoDsIntvFor3DVolume(box, neutube::ONEGIGA / 2, true);
  }

  if (dsIntv > 0) {
    dsObj.downsampleMax(dsIntv, dsIntv, dsIntv);
  }

//  dsObj.fillHole();

  ZMesh *mesh = new ZMesh;

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
