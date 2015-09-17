#include "zmultitaskmanager.h"
#include <iostream>
#include <QThreadPool>

#include "ztask.h"

ZMultiTaskManager::ZMultiTaskManager(QObject *parent) :
  QObject(parent), m_activeTaskNumber(0)
{
  getThreadPool()->setExpiryTimeout(-1);
  getThreadPool()->setMaxThreadCount(QThread::idealThreadCount() - 1);
}

ZMultiTaskManager::~ZMultiTaskManager()
{
  std::cout << "ZMultiTaskManager destropyed." << std::endl;
#if 0
  foreach (ZTask *task, m_taskArray) {
    if (disconnect(task, SIGNAL(finished()), this, SLOT(process()))) {
      std::cout << "Signal-slot disconnected." << std::endl;
    }
  }
#endif
}

bool ZMultiTaskManager::clear()
{
  if (hasActiveTask()) {
    return false;
  }

  foreach (ZTask *task, m_taskArray) {
    delete task;
  }
  m_taskArray.clear();

  return true;
}

QThreadPool* ZMultiTaskManager::getThreadPool() const
{
  return QThreadPool::globalInstance();
}

void ZMultiTaskManager::waitForDone()
{
  getThreadPool()->waitForDone();
}

void ZMultiTaskManager::start()
{
  if (m_taskArray.empty()) {
    return;
  }

#ifdef _DEBUG_
  std::cout << "Start with " << getThreadPool()->maxThreadCount() << " threads."
            << std::endl;
#endif
  startProgress();
  prepare();

  foreach (ZTask *task, m_taskArray) {
#ifdef _DEBUG_2
    std::cout << "Start task" << std::endl;
#endif
    getThreadPool()->start(task);
  }
  m_activeTaskNumber = m_taskArray.size();
}

void ZMultiTaskManager::addTask(ZTask *task)
{
  m_taskArray.append(task);
  task->setParent(this);
  task->prepare();
  connect(task, SIGNAL(finished()), this, SLOT(process()));
#ifdef _DEBUG_2
  connect(task, SIGNAL(finished()), task, SLOT(test()));
#endif
}

void ZMultiTaskManager::process()
{
  if (m_activeTaskNumber < 0) {
    return;
  }

  --m_activeTaskNumber;
#ifdef _DEBUG_
  std::cout << "Active task number " << m_activeTaskNumber << std::endl;
#endif

  advanceProgress(1.0 / m_taskArray.size());
  if (m_activeTaskNumber == 0) {
    postProcess();
    endProgress();
    emit finished();
  }
}



//////////////////////////////////////////////////////////////////
ZSquareTaskManager::ZSquareTaskManager(QObject *parent) :
  ZMultiTaskManager(parent), m_result(0.0)
{
}

void ZSquareTaskManager::prepare()
{
  m_result = 0;
}

void ZSquareTaskManager::postProcess()
{
  foreach (ZTask *task, m_taskArray) {
    ZSquareTask *matchTask =
        dynamic_cast<ZSquareTask*>(task);
    if (matchTask != NULL) {
      m_result += matchTask->getResult();
    }
  }

  std::cout << "Sum of square: " << m_result << std::endl;
}

