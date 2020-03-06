#include "zdvidtracingtask.h"

#include "dvid/zdvidneurontracer.h"
#include "zswctree.h"
#include "mvc/zstackdoc.h"
#include "zstackdocaccessor.h"

ZDvidTracingTask::ZDvidTracingTask()
{
}

void ZDvidTracingTask::setDocument(ZStackDoc *doc)
{
  m_doc = doc;
}

void ZDvidTracingTask::setReader(const ZDvidReader &reader)
{
  if (!m_tracer) {
    m_tracer = std::shared_ptr<ZDvidNeuronTracer>(new ZDvidNeuronTracer);
  }

  m_tracer->setDvidReader(reader);
}

void ZDvidTracingTask::setSeed(double x, double y, double z, double r)
{
  m_x = x;
  m_y = y;
  m_z = z;
  m_r = r;
}

void ZDvidTracingTask::execute()
{
  if (m_tracer) {
    m_tracer->trace(m_x, m_y, m_z, m_r);
    ZSwcTree *tree = m_tracer->getResult();
    if (tree) {
      if (!tree->isEmpty()) {
        ZStackDocAccessor::AddObject(m_doc, tree);
      } else {
        delete tree;
      }
    }
  }
}


