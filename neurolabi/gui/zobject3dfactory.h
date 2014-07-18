#ifndef ZOBJECT3DFACTORY_H
#define ZOBJECT3DFACTORY_H

#include "zobject3d.h"
#include "zobject3darray.h"

class ZStack;

class ZObject3dFactory
{
public:
  ZObject3dFactory();

public:
  static ZObject3dArray* makeRegionBoundary(const ZStack &stack);
};

#endif // ZOBJECT3DFACTORY_H
