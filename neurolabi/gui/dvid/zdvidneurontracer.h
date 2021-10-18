#ifndef ZDVIDNEURONTRACER_H
#define ZDVIDNEURONTRACER_H

#include <list>
#include "zdvidreader.h"
#include "tz_local_neuroseg.h"

class ZSwcTree;
class ZDvidTarget;
class ZIntCuboid;

class ZDvidNeuronTracer
{
public:
  ZDvidNeuronTracer();
  ~ZDvidNeuronTracer();

  void setDvidTarget(const ZDvidTarget &target);
  void setDvidReader(const ZDvidReader &reader);

//  ZSwcTree *takeResult();
  ZSwcTree* getResult() const;

  void trace(double x, double y, double z, double r);
//  void tuneChainEnd();

  double fit(Local_Neuroseg *locseg);

  double optimize(Local_Neuroseg *locseg);

  ZStack* readStack(const ZIntCuboid &box);
  ZStack* readStack(double x, double y, double z, double r);

private:
  void init();

  double getFitScore() const;

  void registerToRawStack(const ZIntPoint &stackOffset, Local_Neuroseg *locseg);
  void registerToStack(const ZIntPoint &stackOffset, Local_Neuroseg *locseg);

  bool hitTraced(const Local_Neuroseg *locseg) const;


private:
  ZDvidReader m_dvidReader;
  ZDvidInfo m_dvidInfo;
//  ZSwcTree *m_resultTree;
  std::list<Local_Neuroseg*> m_locsegList;
  Locseg_Fit_Workspace *m_fitWorkspace;

//  Local_Neuroseg *m_tailSeg = nullptr;
//  Local_Neuroseg *m_headSeg = nullptr;
//  bool m_tuningEnd = true;
  double m_seedMinScore;
  double m_traceMinScore;
};

#endif // ZDVIDNEURONTRACER_H
