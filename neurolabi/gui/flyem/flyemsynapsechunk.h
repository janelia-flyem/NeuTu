#ifndef FLYEMSYNAPSECHUNK_H
#define FLYEMSYNAPSECHUNK_H

#include "bigdata/zintpointannotationchunk.h"
#include "dvid/zdvidsynapse.h"

class FlyEmSynapseChunk : public ZIntPointAnnotationChunk<ZDvidSynapse>
{
public:
  FlyEmSynapseChunk();
};

#endif // FLYEMSYNAPSECHUNK_H
