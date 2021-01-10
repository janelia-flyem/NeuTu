#include "zworkerwrapper.h"

#include <QObject>

#include "zworker.h"
#include "zworkthread.h"
#include "ztask.h"

ZWorkerWrapper::ZWorkerWrapper()
{
}

ZWorkerWrapper::~ZWorkerWrapper()
{
}

void ZWorkerWrapper::endWorkThread()
{
  if (m_workThread) {
    m_workThread->cancelAndQuit();
  }
  m_worker = nullptr;
  m_workThread = nullptr;
}

void ZWorkerWrapper::startWorkThread()
{
  if (m_workThread == nullptr) {
    m_worker = new ZWorker(ZWorker::EMode::SCHEDULE);
    m_workThread = new ZWorkThread(m_worker);
    QObject::connect(
          m_workThread, SIGNAL(finished()), m_workThread, SLOT(deleteLater()));
    m_workThread->start();
  }
}

void ZWorkerWrapper::addTask(ZTask *task)
{
  if (m_workThread) {
    m_workThread->addTask(task);
  } else {
    delete task;
  }
}
