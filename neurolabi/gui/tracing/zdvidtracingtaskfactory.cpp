#include "zdvidtracingtaskfactory.h"

#include "dvid/zdvidneurontracer.h"
#include "zdvidtracingtask.h"

ZDvidTracingTaskFactory::ZDvidTracingTaskFactory()
{
}

void ZDvidTracingTaskFactory::setDocument(ZStackDoc *doc)
{
  m_doc = doc;
}

void ZDvidTracingTaskFactory::setReader(const ZDvidReader &reader)
{
  m_reader = reader;
}

ZTask* ZDvidTracingTaskFactory::makeTask(
    double x, double y, double z, double r) const
{
  ZDvidTracingTask *task = nullptr;

  if (m_doc && m_reader.isReady()) {
    task = new ZDvidTracingTask;
    task->setDocument(m_doc);
    task->setReader(m_reader);
    task->setSeed(x, y, z, r);
  }

  return task;
}

