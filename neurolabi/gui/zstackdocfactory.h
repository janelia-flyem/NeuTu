#ifndef ZSTACKDOCFACTORY_H
#define ZSTACKDOCFACTORY_H

#include "common/zsharedpointer.h"
#include "common/neutube_def.h"

class ZStackDoc;

class ZStackDocFactory
{
public:
  ZStackDocFactory();

public:
  static ZSharedPointer<ZStackDoc> Make(neutu::Document::ETag tag);

};

#endif // ZSTACKDOCFACTORY_H
