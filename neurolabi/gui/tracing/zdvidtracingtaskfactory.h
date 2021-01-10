#ifndef ZDVIDTRACINGTASKFACTORY_H
#define ZDVIDTRACINGTASKFACTORY_H

#include "ztracingtaskfactory.h"
#include "dvid/zdvidreader.h"

class ZStackDoc;

class ZDvidTracingTaskFactory : public ZTracingTaskFactory
{
public:
  ZDvidTracingTaskFactory();
  virtual ~ZDvidTracingTaskFactory() {}

  ZTask* makeTask(double x, double y, double z, double r) const override;

  void setDocument(ZStackDoc *doc);
  void setReader(const ZDvidReader &reader);

private:
  ZStackDoc *m_doc = nullptr;
  ZDvidReader m_reader;
};

#endif // ZDVIDTRACINGTASKFACTORY_H
