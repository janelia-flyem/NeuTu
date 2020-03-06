#include "zworkthread.h"
//#include "logging/zqslog.h"

#include <QTimer>

#include "zworker.h"
#include "ztask.h"

ZWorkThread::ZWorkThread(ZWorker *worker, QObject *parent) : QThread(parent)
{
  setWorker(worker);
}

ZWorkThread::~ZWorkThread()
{
//  LDEBUG() << "Work thread destroyed.";
}


void ZWorkThread::addTask(ZTask *task)
{
  if (m_worker) {
    m_worker->addTask(task);
  } else {
    delete task;
  }
}

void ZWorkThread::setWorker(ZWorker *worker)
{
  m_worker = worker;
  m_worker->moveToThread(this);
  if (m_worker->getMode() == ZWorker::EMode::QUEUE) {
    connect(m_worker, SIGNAL(finished()), this, SLOT(quit()));
    connect(this, SIGNAL(started()), m_worker, SLOT(process()));
  }

  connect(this, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
}

void ZWorkThread::cancelAndQuit()
{
  if (m_worker) {
    m_worker->quit();
  }
  quit();
  wait();
}
