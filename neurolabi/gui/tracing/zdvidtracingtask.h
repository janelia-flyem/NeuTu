#ifndef ZDVIDTRACINGTASK_H
#define ZDVIDTRACINGTASK_H

#include <memory>

#include "ztask.h"

class ZDvidNeuronTracer;
class ZStackDoc;
class ZDvidReader;

class ZDvidTracingTask : public ZTask
{
public:
  ZDvidTracingTask();

  void setDocument(ZStackDoc *doc);
  void setReader(const ZDvidReader &reader);
  void setSeed(double x, double y, double z, double r);

  void execute() override;

private:
  ZStackDoc *m_doc = nullptr;
  std::shared_ptr<ZDvidNeuronTracer> m_tracer;

  double m_x = 0.0;
  double m_y = 0.0;
  double m_z = 0.0;
  double m_r = 3.0;
};

#endif // ZDVIDTRACINGTASK_H
