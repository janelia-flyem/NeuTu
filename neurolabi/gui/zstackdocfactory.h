#ifndef ZSTACKDOCFACTORY_H
#define ZSTACKDOCFACTORY_H

#include "zsharedpointer.h"
#include "neutube_def.h"

class ZStackDoc;

class ZStackDocFactory
{
public:
  ZStackDocFactory();

public:
  static ZSharedPointer<ZStackDoc> Make(NeuTube::Document::ETag tag);

};

#endif // ZSTACKDOCFACTORY_H
