#include "zworkthread.h"
#include "QsLog.h"
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
  connect(m_worker, SIGNAL(finished()), this, SLOT(quit()));
  connect(this, SIGNAL(started()), m_worker, SLOT(process()));
  connect(this, SIGNAL(finished()), m_worker, SLOT(deleteLater()));
}
