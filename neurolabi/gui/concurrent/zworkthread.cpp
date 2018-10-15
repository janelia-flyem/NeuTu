#include "zworkthread.h"
#include "zqslog.h"
#include "zworker.h"

ZWorkThread::ZWorkThread(ZWorker *worker, QObject *parent) : QThread(parent)
{
  setWorker(worker);
}

ZWorkThread::~ZWorkThread()
{
  LDEBUG() << "Work thread destroyed.";
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
