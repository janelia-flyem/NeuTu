#ifndef ZDVIDNEURONTRACER_H
#define ZDVIDNEURONTRACER_H


#include "zdvidreader.h"

class ZSwcTree;
class ZDvidTarget;

class ZDvidNeuronTracer
{
public:
  ZDvidNeuronTracer();

  void setDvidTarget(const ZDvidTarget &target);

  ZSwcTree *getResult() const;

  void trace(int x, int y, int z);

private:
  void init();


private:
  ZDvidReader m_dvidReader;
  ZSwcTree *m_resultTree;
};

#endif // ZDVIDNEURONTRACER_H
