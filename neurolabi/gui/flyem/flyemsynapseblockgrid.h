#ifndef FLYEMSYNAPSEBLOCKGRID_H
#define FLYEMSYNAPSEBLOCKGRID_H

#include "bigdata/zintpointannotationblockgrid.hpp"

#include "dvid/zdvidsynapse.h"
#include "flyemsynapsechunk.h"

class FlyEmSynapseBlockGrid :
    public ZIntPointAnnotationBlockGrid<ZDvidSynapse, FlyEmSynapseChunk>
{
public:
  FlyEmSynapseBlockGrid();
};

#endif // FLYEMSYNAPSEBLOCKGRID_H
