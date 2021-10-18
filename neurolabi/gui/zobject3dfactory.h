#ifndef ZOBJECT3DFACTORY_H
#define ZOBJECT3DFACTORY_H

#include <vector>
#include <map>

#include "common/neutudefs.h"

class ZStack;
class ZObject3d;
class ZObject3dArray;
class ZObject3dScan;
class ZObject3dScanArray;
class ZClosedCurve;
class ZArray;
class ZIntCuboid;
class ZStroke2d;
class ZStackArray;

class ZObject3dFactory
{
public:
  ZObject3dFactory();

public:
  enum EOutputForm {
    OUTPUT_COMPACT, OUTPUT_SPARSE
  };

  static ZObject3dArray* MakeRegionBoundary(
      const ZStack &stack, EOutputForm option);
  static ZObject3dScan MakeObject3dScan(const ZStack &stack);
  static ZObject3dScan* MakeObject3dScan(
      const ZStack &stack, ZObject3dScan *out);
  static ZObject3dScanArray* MakeObject3dScanArray(const ZStack &stack,
                                                   int yStep = 1);
  static ZObject3dScanArray* MakeObject3dScanArray(
      const ZStack &stack, neutu::EAxis sliceAxis);

  static std::vector<ZObject3dScan*> MakeObject3dScanPointerArray(
      const ZStack &stack, int yStep = 1, bool boundaryOnly = true);

  static ZObject3dScanArray* MakeObject3dScanArray(
      ZStackArray &stackArray);

  /*!
   * \brief Extract objects from a stack
   *
   * \param stack Input stack.
   * \param axis Scanning axis.
   * \param foreground Exclude background object if it is true.
   * \param out Output pointer if it is not NULL.
   * \return An array of objects.
   */
  static ZObject3dScanArray* MakeObject3dScanArray(
      const ZStack &stack, neutu::EAxis axis, bool foreground,
      ZObject3dScanArray *out);

  static ZObject3dScanArray* MakeObject3dScanArray(
      const ZArray &array, int yStep, ZObject3dScanArray *out,
      bool foreground);

  static ZObject3dScanArray* MakeObject3dScanArray(
      const ZArray &array, neutu::EAxis axis, bool foreground,
      ZObject3dScanArray *out);

  static ZObject3dScan* MakeObject3dScan(
      const std::vector<ZArray*> labelArray, uint64_t v, ZObject3dScan *out);
  static ZObject3dScan* MakeObject3dScan(
      const std::vector<ZArray*> labelArray, uint64_t v,
      const ZIntCuboid &range, ZObject3dScan *out);

  static ZObject3dScan* MakeFilledMask(const ZClosedCurve &curve, int z,
                                       ZObject3dScan *result = NULL);

  static ZObject3dScan MakeObject3dScan(const ZIntCuboid &box);
  static ZObject3dScan* MakeObject3dScan(
      const ZIntCuboid &box, ZObject3dScan *result);

  static ZStack* MakeBoundaryStack(const ZStack &stack);
  /*!
   * \brief Extracts objects from a stack.
   *
   * \return an array of objects, each representing a region of one value.
   */
  static std::vector<ZObject3d*> MakeObject3dArray(const ZStack &stack);

  static ZObject3dScan MakeRandomObject3dScan(const ZIntCuboid &box);

  static ZObject3dScan MakeObject3dScan(const ZStroke2d &stroke);
  static ZObject3dScan* MakeBoxObject3dScan(
      const ZIntCuboid &box, ZObject3dScan *obj);

  static ZObject3d* MakeBoxObject3d(const ZIntCuboid &box, ZObject3d *obj);

private:
  static void AdjustResolution(
      std::map<uint64_t, ZObject3dScan*> &lowResSet,
      std::map<uint64_t, ZObject3dScan*> &highResSet);
  static std::map<uint64_t, ZObject3dScan*>* ExtractAllForegroundObject(
      ZStack &stack, bool upsampling);
  static void DeleteObjectMap(std::map<uint64_t, ZObject3dScan*> *bodySet);
  static void CombineObjectMap(
      std::map<uint64_t, ZObject3dScan*> *masterBodySet,
      std::map<uint64_t, ZObject3dScan*> *bodySet);
};

#endif // ZOBJECT3DFACTORY_H
