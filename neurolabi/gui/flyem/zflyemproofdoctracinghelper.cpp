#include "zflyemproofdoctracinghelper.h"

#include "geometry/zpoint.h"
#include "geometry/zintpoint.h"

#include "tracing/zdvidtracingtaskfactory.h"
#include "zflyemproofdoc.h"

ZFlyEmProofDocTracingHelper::ZFlyEmProofDocTracingHelper()
{
}

void ZFlyEmProofDocTracingHelper::setDocument(ZFlyEmProofDoc *doc)
{
  m_doc = doc;
  if (!m_taskFactory) {
    m_taskFactory = std::shared_ptr<ZDvidTracingTaskFactory>(
          new ZDvidTracingTaskFactory);
    m_taskFactory->setDocument(m_doc);
    m_taskFactory->setReader(m_doc->getWorkReader());
  }
}

void ZFlyEmProofDocTracingHelper::trace(double x, double y, double z)
{
  if (m_taskFactory) {
    ZTask *task = m_taskFactory->makeTask(x, y, z, 2.0);
    if (task) {
      m_doc->addTask(task);
    }
  }
}

void ZFlyEmProofDocTracingHelper::trace(const ZIntPoint &pt)
{
  trace(pt.getX(), pt.getY(), pt.getZ());
}

void ZFlyEmProofDocTracingHelper::trace(const ZPoint &pt)
{
  trace(pt.getX(), pt.getY(), pt.getZ());
}

bool ZFlyEmProofDocTracingHelper::isReady() const
{
  return m_doc;
}
