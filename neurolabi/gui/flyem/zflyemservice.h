#ifndef ZFLYEMSERVICE_H
#define ZFLYEMSERVICE_H

#include "zintcuboidarray.h"
#include "zobject3dscan.h"
#include "dvid/zdvidtarget.h"
#include "zgraph.h"
#include "zintpairmap.h"
#include "zflyem.h"
#include "flyem/zflyemcoordinateconverter.h"

namespace FlyEm {
namespace Service {
class FaceOrphanOverlap {
public:
  class FaceIndex {
  public:
    FaceIndex();
    FaceIndex(int ci, int fi);

    inline void set(int ci, int fi) {
      m_cuboidIndex = ci;
      m_faceIndex = fi;
    }

    inline int getCuboidIndex() const { return m_cuboidIndex; }
    inline int getFaceIndex() const { return m_faceIndex; }

  private:
    int m_cuboidIndex;
    int m_faceIndex;
  };

  FaceOrphanOverlap();

  void setDvidTarget(const ZDvidTarget &target);
  void loadFace(const ZIntCuboidArray &cuboidArray);
//  void markBody();
  void loadFaceOrphanBody(const std::vector<int> &bodyIdArray);
  ZGraph* computeOverlap();

  std::vector<FaceIndex> getFaceIndexForBody(int bodyIndex) const;
  int getStackIndex(const FaceIndex &faceIndex) const;

  std::vector<ZStack*> getNeighborStackList(int stackIndex) const;

  ZIntPairMap countVoxelTouch(ZStack *stack1, ZStack *stack2,
                              int sourceBodyId);

  inline std::vector<ZIntPoint> getMarker() const {
    return m_marker;
  }

  void print() const;

  void setCoordinateConverter(const ZFlyEmCoordinateConverter &converter);

  void exportJsonFile(const std::string filePath) const;

  void loadSynapse(const std::string &filePath);
#ifdef _DEBUG_
public:
#else
private:
#endif
  void clearFaceStackArray();
  int getLabel(ZStack *stack, int x, int y);
  static ZStack* transformStack(ZStack *stack);
  static void transformCoordinates(
      ZStack *stack, int x, int y, int *cx, int *cy, int *cz);
  static std::pair<int, int> touchTest(const Cuboid_I &c1,
                                       const Cuboid_I &c2);
  static bool hasTouchingFace(const Cuboid_I &c1, const Cuboid_I &c2);
  static void printLabelSet(const ZStack *stack);
  void markBodyMock();
  void loadFaceOrphanBodyMock(const std::vector<int> &bodyIdArray);
  void printMarkerBuffer();

private:
  ZDvidTarget m_dvidTarget;
  std::vector<ZIntCuboidFaceArray> m_faceGroup;
  ZIntCuboidArray m_cuboidArray;
  std::vector<ZStack*> m_faceStackArray;
  std::vector<std::vector<FaceIndex> > m_faceIndexArray;
  std::vector<ZObject3dScan> m_faceOrphanBody;
  ZGraph m_overlapGraph;
  ZGraph m_faceGraph;
  std::vector<int> m_bodyIdArray;
  std::vector<int> m_bodySizeArray;
  std::vector<int> m_synapseCount;
  std::map<std::pair<int, int>, ZIntPoint> m_markerBuffer;
  std::vector<ZIntPoint> m_marker;
  ZFlyEmCoordinateConverter m_coordConverter;
};

}
}

#endif // ZFLYEMSERVICE_H
