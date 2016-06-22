#ifndef ZSWCFACTORY_H
#define ZSWCFACTORY_H

#include "zcuboid.h"
#if defined (_FLYEM_)
#include "flyem/zflyemneuronrange.h"
#include "flyem/zflyemneuronaxis.h"
#endif
#include "zvoxelarray.h"

class ZSwcTree;
class ZPointArray;
class ZLineSegmentArray;
class ZIntCuboidFace;
class ZIntCuboidFaceArray;
class ZStroke2d;
class ZObject3d;
class ZObject3dScan;
class ZStack;
class ZClosedCurve;
class ZDvidInfo;

class ZSwcFactory
{
public:
  ZSwcFactory();

  enum EPostProcess {
    OPTIMAL_SAMPLING, SPARSE_SAMPLING, REGION_SAMPLING, NO_PROCESS
  };

  static ZSwcTree* CreateVirtualRootSwc();
  static ZSwcTree* CreateCircleSwc(double cx, double cy, double cz, double r);
  static ZSwcTree* CreateBoxSwc(const ZCuboid &box, double radius = 1.0);
  static ZSwcTree* CreateBoxSwc(const ZIntCuboid &box, double radius = 1.0);
#if defined (_FLYEM_)
  static ZSwcTree* CreateSwc(const ZFlyEmNeuronRange &range);
  static ZSwcTree* CreateSwc(const ZFlyEmNeuronRange &range,
                             const ZFlyEmNeuronAxis &axis);
  static ZSwcTree* CreateRangeCompareSwc(
      const ZFlyEmNeuronRange &range, const ZFlyEmNeuronRange &reference);
#endif
  static ZSwcTree* CreateSwc(const ZVoxelArray &voxelArray,
                             EPostProcess option = NO_PROCESS);

  static ZSwcTree* CreateSwc(const ZPointArray &pointArray, double radius,
                             bool isConnected = false);

  static ZSwcTree* CreateSwc(const ZLineSegmentArray &lineArray, double radius);

  static ZSwcTree* CreateSwc(const ZIntCuboidFace &face, double radius);
  static ZSwcTree* CreateSwc(const ZIntCuboidFaceArray &faceArray, double radius);

  static ZSwcTree* CreateSwc(const ZStroke2d &stroke);
  static ZSwcTree* CreateSwc(
      const ZObject3d &obj, double radius, int sampleStep);

  static ZSwcTree* CreateSwc(const ZObject3dScan &obj);

  static ZSwcTree* CreateSurfaceSwc(const ZStack &stack, int sparseLevel = 1);
  static ZSwcTree* CreateSurfaceSwc(const ZObject3dScan &obj,
                                    int sparseLevel = 1);

  static ZSwcTree* CreateSwc(const ZClosedCurve &curve, double radius);

  static ZSwcTree* CreateSwc(const ZObject3dScan &blockObj, int z,
                             const ZDvidInfo &dvidInfo);

private:
  static ZSwcTree* CreateSwcByRegionSampling(const ZVoxelArray &voxelArray,
                                             double radiusAdjustment = 0.0);
  static ZSwcTree* CreateSurfaceSwcFast(
      const ZStack &stack, int sparseLevel = 1);
};

#endif // ZSWCFACTORY_H
