#ifndef ZSWCGENERATOR_H
#define ZSWCGENERATOR_H

#include "zcuboid.h"
#include "flyem/zflyemneuronrange.h"
#include "flyem/zflyemneuronaxis.h"
#include "zvoxelarray.h"

class ZSwcTree;
class ZPointArray;
class ZLineSegmentArray;
class ZIntCuboidFace;
class ZIntCuboidFaceArray;
class ZStroke2d;

class ZSwcGenerator
{
public:
  ZSwcGenerator();

  enum EPostProcess {
    OPTIMAL_SAMPLING, SPARSE_SAMPLING, REGION_SAMPLING, NO_PROCESS
  };


  static ZSwcTree* createVirtualRootSwc();
  static ZSwcTree* createCircleSwc(double cx, double cy, double cz, double r);
  static ZSwcTree* createBoxSwc(const ZCuboid &box);
  static ZSwcTree* createSwc(const ZFlyEmNeuronRange &range);
  static ZSwcTree* createSwc(const ZFlyEmNeuronRange &range,
                             const ZFlyEmNeuronAxis &axis);
  static ZSwcTree* createRangeCompareSwc(
      const ZFlyEmNeuronRange &range, const ZFlyEmNeuronRange &reference);

  static ZSwcTree* createSwc(const ZVoxelArray &voxelArray,
                             EPostProcess option = NO_PROCESS);

  static ZSwcTree* createSwc(const ZPointArray &pointArray, double radius,
                             bool isConnected = false);

  static ZSwcTree* createSwc(const ZLineSegmentArray &lineArray, double radius);

  static ZSwcTree* createSwc(const ZIntCuboidFace &face, double radius);
  static ZSwcTree* createSwc(const ZIntCuboidFaceArray &faceArray, double radius);

  static ZSwcTree* createSwc(const ZStroke2d &stroke);

private:
  static ZSwcTree* createSwcByRegionSampling(const ZVoxelArray &voxelArray,
                                             double radiusAdjustment = 0.0);
};

#endif // ZSWCGENERATOR_H
