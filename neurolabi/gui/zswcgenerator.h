#ifndef ZSWCGENERATOR_H
#define ZSWCGENERATOR_H

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

/*!
 * \brief The ZSwcGenerator class
 *
 * Obsolete. Use ZSwcFactory instead.
 */
class ZSwcGenerator
{
public:
  ZSwcGenerator();

  enum EPostProcess {
    OPTIMAL_SAMPLING, SPARSE_SAMPLING, REGION_SAMPLING, NO_PROCESS
  };


  static ZSwcTree* createVirtualRootSwc();
  static ZSwcTree* createCircleSwc(double cx, double cy, double cz, double r);
  static ZSwcTree* createBoxSwc(const ZCuboid &box, double radius = 1.0);
  static ZSwcTree* createBoxSwc(const ZIntCuboid &box, double radius = 1.0);
#if defined (_FLYEM_)
  static ZSwcTree* createSwc(const ZFlyEmNeuronRange &range);
  static ZSwcTree* createSwc(const ZFlyEmNeuronRange &range,
                             const ZFlyEmNeuronAxis &axis);
  static ZSwcTree* createRangeCompareSwc(
      const ZFlyEmNeuronRange &range, const ZFlyEmNeuronRange &reference);
#endif
  static ZSwcTree* createSwc(const ZVoxelArray &voxelArray,
                             EPostProcess option = NO_PROCESS);

  static ZSwcTree* createSwc(const ZPointArray &pointArray, double radius,
                             bool isConnected = false);

  static ZSwcTree* createSwc(const ZLineSegmentArray &lineArray, double radius);

  static ZSwcTree* createSwc(const ZIntCuboidFace &face, double radius);
  static ZSwcTree* createSwc(const ZIntCuboidFaceArray &faceArray, double radius);

  static ZSwcTree* createSwc(const ZStroke2d &stroke);
  static ZSwcTree* createSwc(
      const ZObject3d &obj, double radius, int sampleStep);

  static ZSwcTree* createSwc(const ZObject3dScan &obj);

  static ZSwcTree* createSurfaceSwc(const ZStack &stack, int sparseLevel = 1);
  static ZSwcTree* createSurfaceSwc(const ZObject3dScan &obj,
                                    int sparseLevel = 1);

  static ZSwcTree* createSwc(const ZClosedCurve &curve, double radius);

  static ZSwcTree* createSwc(const ZObject3dScan &blockObj, int z,
                             const ZDvidInfo &dvidInfo);

  static ZSwcTree* createSwcProjection(const ZSwcTree *tree);

private:
  static ZSwcTree* createSwcByRegionSampling(const ZVoxelArray &voxelArray,
                                             double radiusAdjustment = 0.0);
};

#endif // ZSWCGENERATOR_H
