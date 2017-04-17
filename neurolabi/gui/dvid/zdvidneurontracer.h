#ifndef ZDVIDNEURONTRACER_H
#define ZDVIDNEURONTRACER_H


#include "zdvidreader.h"

class ZSwcTree;
class ZDvidTarget;
class ZIntCuboid;

class ZDvidNeuronTracer
{
public:
  ZDvidNeuronTracer();

  void setDvidTarget(const ZDvidTarget &target);

  ZSwcTree *getResult() const;

  void trace(double x, double y, double z, double r);

private:
  void init();

  ZStack* readStack(const ZIntCuboid &box);


private:
  ZDvidReader m_dvidReader;
  ZSwcTree *m_resultTree;
};

#endif // ZDVIDNEURONTRACER_H
