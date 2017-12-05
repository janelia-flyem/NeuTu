#ifndef ZFLYEMMESHFACTORY_H
#define ZFLYEMMESHFACTORY_H

#include "tz_stdint.h"

#include "zmeshfactory.h"

class ZFlyEmMeshFactory : public ZMeshFactory
{
public:
  ZFlyEmMeshFactory();

  virtual ZMesh* makeMesh(uint64_t bodyId) const;
};

#endif // ZFLYEMMESHFACTORY_H
