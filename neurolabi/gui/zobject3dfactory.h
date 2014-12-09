#ifndef ZOBJECT3DFACTORY_H
#define ZOBJECT3DFACTORY_H

class ZStack;
class ZObject3d;
class ZObject3dArray;
class ZObject3dScan;
class ZObject3dScanArray;

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
  static ZObject3dScanArray* MakeObject3dScanArray(const ZStack &stack);
};

#endif // ZOBJECT3DFACTORY_H
