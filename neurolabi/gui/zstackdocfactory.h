#ifndef ZSTACKDOCFACTORY_H
#define ZSTACKDOCFACTORY_H

#include "core/zsharedpointer.h"
#include "core/neutube_def.h"

class ZStackDoc;

class ZStackDocFactory
{
public:
  ZStackDocFactory();

public:
  static ZSharedPointer<ZStackDoc> Make(neutube::Document::ETag tag);

};

#endif // ZSTACKDOCFACTORY_H
