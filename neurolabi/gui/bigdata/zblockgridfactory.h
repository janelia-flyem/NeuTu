#ifndef ZBLOCKGRIDFACTORY_H
#define ZBLOCKGRIDFACTORY_H

#include "bigdata/zblockgrid.h"
#include "bigdata/zstackblockgrid.h"
#include "zobject3dscan.h"

class ZBlockGridFactory
{
public:
  ZBlockGridFactory();

  ZStackBlockGrid* createStackBlockGrid(const ZObject3dScan &obj);
};

#endif // ZBLOCKGRIDFACTORY_H
