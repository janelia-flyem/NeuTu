#ifndef FLYEMSYNAPSESOURCE_H
#define FLYEMSYNAPSESOURCE_H

#include "bigdata/zintpointannotationsource.hpp"
#include "dvid/zdvidsynapse.h"

class FlyEmSynapseSource : public ZIntPointAnnotationSource<ZDvidSynapse>
{
public:
  FlyEmSynapseSource();
};

#endif // FLYEMSYNAPSESOURCE_H
