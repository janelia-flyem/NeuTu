#include "zworker.h"

#include "QsLog.h"
#include "ztask.h"
#include "ztaskqueue.h"

ZWorker::ZWorker(QObject *parent) : QObject(parent)
{
  m_taskQueue = new ZTaskQueue(this);
}

ZWorker::~ZWorker()
{
  LDEBUG() << "Worker destroyed.";
}

void ZWorker::quit()
{
  addTask(NULL);
  m_quiting = true;
}

void ZWorker::process()
{
  LDEBUG() << "Worker started";


  while (1) {
    ZTask *task = m_taskQueue->get();
    if (m_quiting) {
      break;
    }

    if (task != NULL) {
      task->run();
    } else {
      break;
    }
  }

  LDEBUG() << "Worker finished";

  emit finished();
}

void ZWorker::addTask(ZTask *task)
{
  m_taskQueue->add(task);
}
