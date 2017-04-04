#include "zdvidneurontracer.h"

ZDvidNeuronTracer::ZDvidNeuronTracer()
{
  init();
}

void ZDvidNeuronTracer::init()
{
  m_resultTree = NULL;
}

ZSwcTree* ZDvidNeuronTracer::getResult() const
{
  return m_resultTree;
}
