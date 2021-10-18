#ifndef FLYEMSYNAPSEENSEMBLE_H
#define FLYEMSYNAPSEENSEMBLE_H

#include "dvid/zdvidsynapse.h"
#include "flyemsynapsechunk.h"
#include "flyempointannotationensemble.hpp"

class ZDvidTarget;

class FlyEmSynapseEnsemble :
    public FlyEmPointAnnotationEnsemble<ZDvidSynapse, FlyEmSynapseChunk>
{
public:
  FlyEmSynapseEnsemble();

  static ZStackObject::EType GetType() {
    return ZStackObject::EType::FLYEM_SYNAPSE_ENSEMBLE;
  }

  void setDvidTarget(const ZDvidTarget &target);
};

#endif // FLYEMSYNAPSEENSEMBLE_H
