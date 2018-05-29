#include "zworker.h"

#include "QsLog.h"
#include "ztask.h"
#include "ztaskqueue.h"

ZWorker::ZWorker(QObject *parent) : QObject(parent)
{
  m_taskQueue = new ZTaskQueue(this);
}
/*
void ZWorker::setTaskQueue(ZTaskQueue *queue)
{
  m_taskQueue = queue;
}
*/

void ZWorker::process()
{
  LDEBUG() << "Worker started";

  while (!m_quiting) {
    if (m_taskQueue != NULL) {
      ZTask *task = m_taskQueue->get();
      if (task != NULL) {
        task->run();
      }
    }
  }

  emit finished();
}
