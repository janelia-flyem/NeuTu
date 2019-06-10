#ifndef ZFLYEMMESHFACTORY_H
#define ZFLYEMMESHFACTORY_H

#include <string>

#include "tz_stdint.h"

#include "zmeshfactory.h"

class ZDvidReader;

class ZFlyEmMeshFactory : public ZMeshFactory
{
public:
  ZFlyEmMeshFactory();

  virtual ZMesh* makeMesh(uint64_t bodyId) const;

  static ZMesh* MakeRoiMesh(
      const ZDvidReader &reader, const std::string &roiName);
};

#endif // ZFLYEMMESHFACTORY_H
