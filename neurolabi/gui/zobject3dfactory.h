#ifndef ZOBJECT3DFACTORY_H
#define ZOBJECT3DFACTORY_H

class ZStack;
class ZObject3d;
class ZObject3dArray;
class ZObject3dScan;

class ZObject3dFactory
{
public:
  ZObject3dFactory();

public:
  static ZObject3dArray* makeRegionBoundary(const ZStack &stack);
  static ZObject3dScan makeObject3dScan(const ZStack &stack);
  static ZObject3dScan* makeObject3dScan(
      const ZStack &stack, ZObject3dScan *out);
};

#endif // ZOBJECT3DFACTORY_H
